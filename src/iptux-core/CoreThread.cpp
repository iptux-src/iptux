#include "config.h"
#include "iptux-core/CoreThread.h"

#include "Const.h"
#include <cstdio>
#include <deque>
#include <fstream>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>

#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <poll.h>
#include <sys/stat.h>

#include "iptux-core/Exception.h"
#include "iptux-core/internal/Command.h"
#include "iptux-core/internal/RecvFileData.h"
#include "iptux-core/internal/SendFile.h"
#include "iptux-core/internal/TcpData.h"
#include "iptux-core/internal/UdpData.h"
#include "iptux-core/internal/UdpDataService.h"
#include "iptux-core/internal/ipmsg.h"
#include "iptux-core/internal/support.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"
#include <unistd.h>

using namespace std;
using namespace std::placeholders;

namespace iptux {

namespace {
/**
 * 初始化程序iptux的运行环境.
 * cache iptux {pic, photo, icon} \n
 * config iptux {log, photo, icon} \n
 */
void init_iptux_environment() {
  const char* env;
  char path[MAX_PATHLEN];

  env = g_get_user_cache_dir();
  if (access(env, F_OK) != 0)
    g_mkdir(env, 0755);
  snprintf(path, MAX_PATHLEN, "%s" IPTUX_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0755);
  snprintf(path, MAX_PATHLEN, "%s" PIC_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0755);
  snprintf(path, MAX_PATHLEN, "%s" PHOTO_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0755);
  snprintf(path, MAX_PATHLEN, "%s" ICON_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0755);
  snprintf(path, MAX_PATHLEN, "%s" LOG_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0755);
  snprintf(path, MAX_PATHLEN, "%s" SENT_IMAGE_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0755);

  env = g_get_user_config_dir();
  if (access(env, F_OK) != 0)
    g_mkdir(env, 0777);
  snprintf(path, MAX_PATHLEN, "%s" IPTUX_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0777);
  snprintf(path, MAX_PATHLEN, "%s" LOG_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0777);
  snprintf(path, MAX_PATHLEN, "%s" PHOTO_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0777);
  snprintf(path, MAX_PATHLEN, "%s" ICON_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0777);
  snprintf(path, MAX_PATHLEN, "%s" LOG_PATH, env);
  if (access(path, F_OK) != 0)
    g_mkdir(path, 0777);
}

}  // namespace

struct CoreThread::Impl {
  uint16_t port;

  PPalInfo me;

  UdpDataService_U udp_data_service;
  uint8_t encrypt_msg : 1;

  GSList* blacklist{nullptr};  // 黑名单链表
  bool debugDontBroadcast{false};
  vector<shared_ptr<PalInfo>> pallist;  // 好友链表(成员不能被删除)

  map<uint32_t, shared_ptr<FileInfo>> privateFiles;
  int lastTransTaskId{0};
  int eventCount{0};
  shared_ptr<const Event> lastEvent{nullptr};
  map<int, shared_ptr<TransAbstract>> transTasks;
  deque<shared_ptr<const Event>> waitingEvents;
  std::mutex waitingEventsMutex;

  future<void> udpFuture;
  future<void> tcpFuture;
  future<void> notifyToAllFuture;
};

CoreThread::CoreThread(shared_ptr<ProgramData> data)
    : programData(data),
      config(data->getConfig()),
      tcpSock(-1),
      udpSock(-1),
      started(false),
      pImpl(std::make_unique<Impl>()) {
  if (config->GetBool("debug_dont_broadcast")) {
    pImpl->debugDontBroadcast = true;
  }
  pImpl->port = programData->port();
  pImpl->udp_data_service = make_unique<UdpDataService>(*this);
  pImpl->me = make_shared<PalInfo>("127.0.0.1", port());
  (*pImpl->me)
      .setUser(g_get_user_name())
      .setHost(g_get_host_name())
      .setName(programData->nickname)
      .setGroup(programData->mygroup)
      .setEncode("utf-8")
      .setCompatible(true);

  if (programData->isEncryptMsg()) {
    if (!programData->initPrivateKey()) {
      LOG_ERROR("CoreThread: Failed to initialize encryption keys.");
    } else {
      pImpl->encrypt_msg = true;
    }
  }
}

CoreThread::~CoreThread() {
  if (started) {
    stop();
  }
  g_slist_free(pImpl->blacklist);
}

/**
 * 程序核心入口，主要任务服务将在此开启.
 */
void CoreThread::start() {
  if (started) {
    throw "CoreThread already started, can't start twice";
  }
  started = true;
  init_iptux_environment();
  bind_iptux_port();

  pImpl->udpFuture = async([](CoreThread* ct) { RecvUdpData(ct); }, this);
  pImpl->tcpFuture = async([](CoreThread* ct) { RecvTcpData(ct); }, this);
  pImpl->notifyToAllFuture =
      async([](CoreThread* ct) { SendNotifyToAll(ct); }, this);
}

void CoreThread::bind_iptux_port() {
  uint16_t port = programData->port();
  struct sockaddr_in addr;
  tcpSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_enable_reuse(tcpSock);
  udpSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  socket_enable_reuse(udpSock);
  socket_enable_broadcast(udpSock);
  if ((tcpSock == -1) || (udpSock == -1)) {
    int ec = errno;
    const char* errmsg = g_strdup_printf(
        _("Fatal Error!! Failed to create new socket!\n%s"), strerror(ec));
    LOG_WARN("%s", errmsg);
    throw Exception(SOCKET_CREATE_FAILED, errmsg);
  }

  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  auto bind_ip = config->GetString("bind_ip", "0.0.0.0");
  addr.sin_addr = inAddrFromString(bind_ip);
  if (::bind(tcpSock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    int ec = errno;
    close(tcpSock);
    close(udpSock);
    auto errmsg =
        stringFormat(_("Fatal Error!! Failed to bind the TCP port(%s:%d)!\n%s"),
                     bind_ip.c_str(), port, strerror(ec));
    LOG_ERROR("%s", errmsg.c_str());
    throw Exception(TCP_BIND_FAILED, errmsg);
  } else {
    LOG_INFO("bind TCP port(%s:%d) success.", bind_ip.c_str(), port);
  }

  if (::bind(udpSock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    int ec = errno;
    close(tcpSock);
    close(udpSock);
    auto errmsg =
        stringFormat(_("Fatal Error!! Failed to bind the UDP port(%s:%d)!\n%s"),
                     bind_ip.c_str(), port, strerror(ec));
    LOG_ERROR("%s", errmsg.c_str());
    throw Exception(UDP_BIND_FAILED, errmsg);
  } else {
    LOG_INFO("bind UDP port(%s:%d) success.", bind_ip.c_str(), port);
  }
}

/**
 * 监听UDP服务端口.
 * @param pcthrd 核心类
 */
void CoreThread::RecvUdpData(CoreThread* self) {
  struct sockaddr_in addr;
  socklen_t len;
  char buf[MAX_UDPLEN];
  ssize_t size;

  while (self->started) {
    struct pollfd pfd = {self->udpSock, POLLIN, 0};
    int ret = poll(&pfd, 1, 10);
    if (ret == -1) {
      LOG_ERROR("poll udp socket failed: %s", strerror(errno));
      return;
    }
    if (ret == 0) {
      continue;
    }
    g_assert(ret == 1);
    len = sizeof(addr);
    if ((size = recvfrom(self->udpSock, buf, MAX_UDPLEN, 0,
                         (struct sockaddr*)&addr, &len)) == -1)
      continue;
    if (size != MAX_UDPLEN)
      buf[size] = '\0';
    auto port = ntohs(addr.sin_port);
    self->pImpl->udp_data_service->process(addr.sin_addr, port, buf, size);
  }
}

/**
 * 监听TCP服务端口.
 * @param pcthrd 核心类
 */
void CoreThread::RecvTcpData(CoreThread* pcthrd) {
  int subsock;

  listen(pcthrd->tcpSock, 5);
  while (pcthrd->started) {
    struct pollfd pfd = {pcthrd->tcpSock, POLLIN, 0};
    int ret = poll(&pfd, 1, 10);
    if (ret == -1) {
      LOG_ERROR("poll udp socket failed: %s", strerror(errno));
      return;
    }
    if (ret == 0) {
      continue;
    }
    g_assert(ret == 1);
    if ((subsock = accept(pcthrd->tcpSock, NULL, NULL)) == -1)
      continue;
    thread([](CoreThread* coreThread,
              int subsock) { TcpData::TcpDataEntry(coreThread, subsock); },
           pcthrd, subsock)
        .detach();
  }
}

void CoreThread::stop() {
  if (!started) {
    throw "CoreThread not started, or already stopped";
  }
  started = false;
  ClearSublayer();
  pImpl->udpFuture.wait();
  pImpl->tcpFuture.wait();
  pImpl->notifyToAllFuture.wait();
}

uint16_t CoreThread::port() const {
  return pImpl->port;
}

void CoreThread::ClearSublayer() {
  /**
   * @note 必须在发送下线信息之后才能关闭套接口.
   */
  for (auto palInfo : pImpl->pallist) {
    SendBroadcastExit(palInfo);
  }
  shutdown(tcpSock, SHUT_RDWR);
  shutdown(udpSock, SHUT_RDWR);
}

int CoreThread::getUdpSock() const {
  return udpSock;
}

shared_ptr<ProgramData> CoreThread::getProgramData() {
  return programData;
}

/**
 * 向局域网内所有计算机发送上线信息.
 * @param pcthrd 核心类
 */
void CoreThread::SendNotifyToAll(CoreThread* pcthrd) {
  Command cmd(*pcthrd);
  if (!pcthrd->pImpl->debugDontBroadcast) {
    cmd.BroadCast(pcthrd->udpSock, pcthrd->port());
  }
  cmd.DialUp(pcthrd->udpSock, pcthrd->port());
}

/**
 * 黑名单链表中是否包含此项.
 * @param ipv4 ipv4
 * @return 是否包含
 */
bool CoreThread::BlacklistContainItem(in_addr ipv4) const {
  return g_slist_find(pImpl->blacklist, GUINT_TO_POINTER(ipv4.s_addr));
}

bool CoreThread::IsBlocked(in_addr ipv4) const {
  return programData->IsUsingBlacklist() and BlacklistContainItem(ipv4);
}

void CoreThread::Lock() const {
  mutex.lock();
}

void CoreThread::Unlock() const {
  mutex.unlock();
}

/**
 * 获取好友链表.
 * @return 好友链表
 */
const vector<shared_ptr<PalInfo>>& CoreThread::GetPalList() {
  return pImpl->pallist;
}

/**
 * 从好友链表中移除所有好友数据(非UI线程安全).
 * @note 鉴于好友链表成员不能被删除，所以将成员改为下线标记即可
 */
void CoreThread::ClearAllPalFromList() {
  /* 清除所有好友的在线标志 */
  for (auto palInfo : pImpl->pallist) {
    palInfo->setOnline(false);
  }
}

shared_ptr<PalInfo> CoreThread::GetPal(PalKey palKey) {
  for (auto palInfo : pImpl->pallist) {
    if (ipv4Equal(palInfo->ipv4(), palKey.GetIpv4())) {
      return palInfo;
    }
  }
  return {};
}

shared_ptr<PalInfo> CoreThread::GetPal(const string& ipv4) {
  return GetPal(PalKey(inAddrFromString(ipv4), port()));
}

/**
 * 从好友链表中删除指定的好友信息数据(非UI线程安全).
 * @param ipv4 ipv4
 * @note 鉴于好友链表成员不能被删除，所以将成员改为下线标记即可；
 * 鉴于群组中只能包含在线的好友，所以若某群组中包含了此好友，则必须从此群组中删除此好友
 */
void CoreThread::DelPalFromList(PalKey palKey) {
  PPalInfo pal;

  /* 获取好友信息数据，并将其置为下线状态 */
  if (!(pal = GetPal(palKey)))
    return;
  pal->setOnline(false);
  emitEvent(make_shared<const PalOfflineEvent>(palKey));
}

/**
 * 通告指定的好友信息数据已经被更新(非UI线程安全).
 * @param ipv4 ipv4
 * @note 什么时候会用到？1、好友更新个人资料；2、好友下线后又上线了
 * @note 鉴于群组中必须包含所有属于自己的成员，移除不属于自己的成员，
 * 所以好友信息更新后应该重新调整群组成员；
 * @note 群组中被更新的成员信息也应该在界面上做出相应更新
 */
void CoreThread::UpdatePalToList(PalKey palKey) {
  PPalInfo pal;
  /* 如果好友链表中不存在此好友，则视为程序设计出错 */
  if (!(pal = GetPal(palKey))) {
    return;
  }
  pal->setOnline(true);
  emitEvent(make_shared<const PalUpdateEvent>(pal));
}

/**
 * 将好友信息数据加入到好友链表(非UI线程安全).
 * @param pal class PalInfo
 * @note 鉴于在线的好友必须被分配到它所属的群组，所以加入好友到好友链表的同时
 * 也应该分配好友到相应的群组
 */
void CoreThread::AttachPalToList(shared_ptr<PalInfo> pal) {
  pImpl->pallist.push_back(pal);
  pal->setOnline(true);
  emitNewPalOnline(pal);
}

void CoreThread::emitNewPalOnline(PPalInfo palInfo) {
  emitEvent(make_shared<NewPalOnlineEvent>(palInfo));
}

void CoreThread::emitNewPalOnline(const PalKey& palKey) {
  auto palInfo = GetPal(palKey);
  if (palInfo) {
    NewPalOnlineEvent event(palInfo);
    emitEvent(make_shared<NewPalOnlineEvent>(palInfo));
  } else {
    LOG_ERROR("emitNewPalOnline meet a unknown key: %s",
              palKey.ToString().c_str());
  }
}

void CoreThread::emitEvent(shared_ptr<const Event> event) {
  lock_guard<std::mutex> l(pImpl->waitingEventsMutex);
  pImpl->waitingEvents.push_back(event);
  this->pImpl->eventCount++;
  this->pImpl->lastEvent = event;
  signalEvent.emit(event);
}

/**
 * 向好友发送iptux特有的数据.
 * @param pal class PalInfo
 */
void CoreThread::sendFeatureData(PPalInfo pal) {
  Command cmd(*this);
  char path[MAX_PATHLEN];
  const gchar* env;
  int sock;

  if (!programData->sign.empty()) {
    cmd.SendMySign(udpSock, pal);
  }
  env = g_get_user_config_dir();
  snprintf(path, MAX_PATHLEN, "%s" ICON_PATH "/%s", env,
           programData->myicon.c_str());
  if (access(path, F_OK) == 0) {
    ifstream ifs(path);
    cmd.SendMyIcon(udpSock, pal, ifs);
  }
  snprintf(path, MAX_PATHLEN, "%s" PHOTO_PATH "/photo", env);
  if (access(path, F_OK) == 0) {
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
      LOG_ERROR(_("Fatal Error!!\nFailed to create new socket!\n%s"),
                strerror(errno));
      throw Exception(CREATE_TCP_SOCKET_FAILED);
    }
    cmd.SendSublayer(sock, pal, IPTUX_PHOTOPICOPT, path);
    close(sock);
  }
}

void CoreThread::SendMyIcon(PPalInfo pal, istream& iss) {
  Command(*this).SendMyIcon(udpSock, pal, iss);
}

void CoreThread::AddBlockIp(in_addr ipv4) {
  pImpl->blacklist =
      g_slist_append(pImpl->blacklist, GUINT_TO_POINTER(ipv4.s_addr));
}

bool CoreThread::SendMessage(CPPalInfo palInfo, const string& message) {
  Command cmd(*this);
  cmd.SendMessage(getUdpSock(), palInfo, message.c_str());
  return true;
}

bool CoreThread::SendMessage(CPPalInfo pal, const ChipData& chipData) {
  auto ptr = chipData.data.c_str();
  switch (chipData.type) {
    case MessageContentType::STRING:
      /* 文本类型 */
      return SendMessage(pal, chipData.data);
    case MESSAGE_CONTENT_TYPE_PICTURE:
      /* 图片类型 */
      int sock;
      if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        LOG_ERROR(_("Fatal Error!!\nFailed to create new socket!\n%s"),
                  strerror(errno));
        return false;
      }
      Command(*this).SendSublayer(sock, pal, IPTUX_MSGPICOPT, ptr);
      close(sock);  // 关闭网络套接口
      return true;
    default:
      g_assert_not_reached();
  }
}

bool CoreThread::SendMsgPara(shared_ptr<MsgPara> para) {
  for (int i = 0; i < int(para->dtlist.size()); ++i) {
    if (!SendMessage(para->getPal(), para->dtlist[i])) {
      LOG_ERROR("send message failed: %s", para->dtlist[i].ToString().c_str());
      return false;
    }
  }
  return true;
}

void CoreThread::AsyncSendMsgPara(std::shared_ptr<MsgPara> msgPara) {
  thread t(&CoreThread::SendMsgPara, this, msgPara);
  t.detach();
}

void CoreThread::InsertMessage(const MsgPara& para) {
  MsgPara para2 = para;
  this->emitEvent(make_shared<NewMessageEvent>(std::move(para2)));
}

void CoreThread::InsertMessage(MsgPara&& para) {
  this->emitEvent(make_shared<NewMessageEvent>(std::move(para)));
}

bool CoreThread::SendAskShared(PPalInfo pal) {
  Command(*this).SendAskShared(getUdpSock(), pal, 0, NULL);
  return true;
}

void CoreThread::SendSharedFiles(PPalInfo pal) {
  SendFile::SendSharedInfoEntry(this, pal);
}

void CoreThread::UpdateMyInfo() {
  Command cmd(*this);

  Lock();
  for (auto pal : pImpl->pallist) {
    if (pal->isOnline()) {
      cmd.SendAbsence(udpSock, pal);
    }
    if (pal->isOnline() and pal->isCompatible()) {
      thread t1(bind(&CoreThread::sendFeatureData, this, _1), pal);
      t1.detach();
    }
  }
  Unlock();
  emitEvent(make_shared<const ConfigChangedEvent>());
}

/**
 * 发送通告本计算机下线的信息.
 * @param pal class PalInfo
 */
void CoreThread::SendBroadcastExit(PPalInfo pal) {
  Command cmd(*this);
  cmd.SendExit(udpSock, pal);
}

int CoreThread::GetOnlineCount() const {
  int res = 0;
  for (auto pal : pImpl->pallist) {
    if (pal->isOnline()) {
      res++;
    }
  }
  return res;
}

void CoreThread::SendDetectPacket(const string& ipv4) {
  SendDetectPacket(inAddrFromString(ipv4));
}

void CoreThread::SendDetectPacket(in_addr ipv4) {
  Command(*this).SendDetectPacket(udpSock, ipv4, port());
}

void CoreThread::emitSomeoneExit(const PalKey& palKey) {
  if (!GetPal(palKey)) {
    return;
  }
  DelPalFromList(palKey);
  emitEvent(make_shared<PalOfflineEvent>(palKey));
}

void CoreThread::EmitIconUpdate(const PalKey& palKey) {
  UpdatePalToList(palKey);
  emitEvent(make_shared<IconUpdateEvent>(palKey));
}

void CoreThread::SendExit(PPalInfo palInfo) {
  Command(*this).SendExit(udpSock, palInfo);
}

const string& CoreThread::GetAccessPublicLimit() const {
  return programData->GetPasswd();
}

void CoreThread::SetAccessPublicLimit(const string& val) {
  programData->SetPasswd(val);
}

void CoreThread::AddPrivateFile(PFileInfo file) {
  g_assert(file);
  g_assert(file->fileid >= MAX_SHAREDFILE);
  g_assert(pImpl->privateFiles.count(file->fileid) == 0);
  pImpl->privateFiles[file->fileid] = file;
}

bool CoreThread::DelPrivateFile(uint32_t id) {
  return pImpl->privateFiles.erase(id) >= 1;
}

PFileInfo CoreThread::GetPrivateFileById(uint32_t id) {
  if (id < MAX_SHAREDFILE) {
    FileInfo* f = programData->GetShareFileInfo(id);
    if (!f)
      return PFileInfo();
    return make_shared<FileInfo>(*f);
  }

  auto res = pImpl->privateFiles.find(id);
  if (res == pImpl->privateFiles.end()) {
    return PFileInfo();
  }
  return res->second;
}

PFileInfo CoreThread::GetPrivateFileByPacketN(uint32_t packageNum,
                                              uint32_t filectime) {
  for (auto& i : pImpl->privateFiles) {
    if (i.second->packetn == packageNum && i.second->filenum == filectime) {
      return i.second;
    }
  }
  return PFileInfo();
}

void CoreThread::RegisterTransTask(std::shared_ptr<TransAbstract> task) {
  int taskId = ++(pImpl->lastTransTaskId);
  task->SetTaskId(taskId);
  pImpl->transTasks[taskId] = task;
  LOG_INFO("add trans task %d", taskId);
}

bool CoreThread::TerminateTransTask(int taskId) {
  auto task = pImpl->transTasks.find(taskId);
  if (task == pImpl->transTasks.end()) {
    return false;
  }
  task->second->TerminateTrans();
  return true;
}

void CoreThread::RecvFile(FileInfo* file) {
  auto rfdt = make_shared<RecvFileData>(this, file);
  RegisterTransTask(rfdt);
  rfdt->RecvFileDataEntry();
}

void CoreThread::RecvFileAsync(FileInfo* file) {
  thread t(&CoreThread::RecvFile, this, file);
  t.detach();
}

std::unique_ptr<TransFileModel> CoreThread::GetTransTaskStat(int taskId) const {
  auto task = pImpl->transTasks.find(taskId);
  if (task == pImpl->transTasks.end()) {
    return {};
  }
  return make_unique<TransFileModel>(task->second->getTransFileModel());
}

void CoreThread::clearFinishedTransTasks() {
  Lock();
  bool changed = false;
  for (auto it = pImpl->transTasks.begin(); it != pImpl->transTasks.end();) {
    if (it->second->getTransFileModel().isFinished()) {
      it = pImpl->transTasks.erase(it);
      changed = true;
    } else {
      it++;
    }
  }
  Unlock();

  if (changed) {
    emitEvent(make_shared<TransTasksChangedEvent>());
  }
}

bool CoreThread::SendAskSharedWithPassword(const PalKey& palKey,
                                           const std::string& password) {
  auto epasswd =
      g_base64_encode((const guchar*)(password.c_str()), password.size());
  Command(*this).SendAskShared(udpSock, palKey, IPTUX_PASSWDOPT, epasswd);
  g_free(epasswd);
  return true;
}

void CoreThread::SendUnitMessage(const PalKey& palKey,
                                 uint32_t opttype,
                                 const string& message) {
  Command(*this).SendUnitMsg(udpSock, GetPal(palKey), opttype, message.c_str());
}

void CoreThread::SendGroupMessage(const PalKey& palKey,
                                  const std::string& message) {
  Command(*this).SendGroupMsg(udpSock, GetPal(palKey), message.c_str());
}

void CoreThread::BcstFileInfoEntry(const vector<const PalInfo*>& pals,
                                   const vector<FileInfo*>& files) {
  SendFile::BcstFileInfoEntry(this, pals, files);
}

vector<unique_ptr<TransFileModel>> CoreThread::listTransTasks() const {
  vector<unique_ptr<TransFileModel>> res;
  Lock();
  for (auto it = pImpl->transTasks.begin(); it != pImpl->transTasks.end();
       it++) {
    res.push_back(make_unique<TransFileModel>(it->second->getTransFileModel()));
  }
  Unlock();
  return res;
}

int CoreThread::getEventCount() const {
  return this->pImpl->eventCount;
}

shared_ptr<const Event> CoreThread::getLastEvent() const {
  return this->pImpl->lastEvent;
}

PPalInfo CoreThread::getMe() {
  return this->pImpl->me;
}

string CoreThread::getUserIconPath() const {
  return stringFormat("%s%s", g_get_user_cache_dir(), ICON_PATH);
}

bool CoreThread::HasEvent() const {
  lock_guard<std::mutex> l(pImpl->waitingEventsMutex);
  return !this->pImpl->waitingEvents.empty();
}

shared_ptr<const Event> CoreThread::PopEvent() {
  lock_guard<std::mutex> l(pImpl->waitingEventsMutex);
  auto event = pImpl->waitingEvents.front();
  pImpl->waitingEvents.pop_front();
  return event;
}

}  // namespace iptux
