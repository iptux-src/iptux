#include "config.h"
#include "CoreThread.h"

#include <thread>
#include <functional>
#include <fstream>
#include <memory>
#include <future>

#include <gio/gio.h>
#include <glib/gi18n.h>
#include <glog/logging.h>

#include "iptux-core/ipmsg.h"
#include "support.h"
#include "iptux-core/output.h"
#include "utils.h"
#include "UdpData.h"
#include "TcpData.h"
#include "Command.h"
#include "deplib.h"
#include "iptux-core/Exception.h"
#include "iptux-core/SendFile.h"
#include "iptux/RecvFileData.h"

using namespace std;
using namespace std::placeholders;

namespace iptux {

struct CoreThread::Impl {
  GSList *blacklist {nullptr};                              //黑名单链表
  bool debugDontBroadcast {false} ;
  vector<shared_ptr<PalInfo>> pallist;  //好友链表(成员不能被删除)

  map<uint32_t, shared_ptr<FileInfo>> privateFiles;
  int lastTransTaskId { 0 };
  map<int, shared_ptr<TransAbstract>> transTasks;

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
      pImpl(std::make_unique<Impl>())
{
  pthread_mutex_init(&mutex, NULL);
  if(config->GetBool("debug_dont_broadcast")) {
    pImpl->debugDontBroadcast = true;
  }
}

CoreThread::~CoreThread() {
  if(started) {
    stop();
  }
  g_slist_free(pImpl->blacklist);
}

/**
 * 程序核心入口，主要任务服务将在此开启.
 */
void CoreThread::start() {
  if(started) {
    throw "CoreThread already started, can't start twice";
  }
  started = true;

  bind_iptux_port();

  pImpl->udpFuture = async([](CoreThread* ct){
    RecvUdpData(ct);
  }, this);
  pImpl->tcpFuture = async([](CoreThread* ct){
    RecvTcpData(ct);
  }, this);
  pImpl->notifyToAllFuture = async([](CoreThread* ct){
    SendNotifyToAll(ct);
  }, this);
}

void CoreThread::bind_iptux_port() {
  int port = config->GetInt("port", IPTUX_DEFAULT_PORT);
  struct sockaddr_in addr;
  tcpSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_enable_reuse(tcpSock);
  udpSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  socket_enable_reuse(udpSock);
  socket_enable_broadcast(udpSock);
  if ((tcpSock == -1) || (udpSock == -1)) {
    int ec = errno;
    const char* errmsg = g_strdup_printf(_("Fatal Error!! Failed to create new socket!\n%s"),
                                         strerror(ec));
    LOG_WARN("%s", errmsg);
    throw BindFailedException(ec, errmsg);
  }

  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  auto bind_ip = config->GetString("bind_ip", "0.0.0.0");
  addr.sin_addr.s_addr = stringToInAddr(bind_ip);
  if (::bind(tcpSock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    int ec = errno;
    close(tcpSock);
    close(udpSock);
    const char* errmsg = g_strdup_printf(_("Fatal Error!! Failed to bind the TCP port(%s:%d)!\n%s"),
                                         bind_ip.c_str(), port, strerror(ec));
    LOG_WARN("%s", errmsg);
    throw BindFailedException(ec, errmsg);
  } else {
    LOG_INFO("bind TCP port(%s:%d) success.", bind_ip.c_str(), port);
  }

  if(::bind(udpSock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    int ec = errno;
    close(tcpSock);
    close(udpSock);
    const char* errmsg = g_strdup_printf(_("Fatal Error!! Failed to bind the UDP port(%s:%d)!\n%s"),
                                         bind_ip.c_str(), port, strerror(ec));
    LOG_WARN("%s", errmsg);
    throw BindFailedException(ec, errmsg);
  } else {
    LOG_INFO("bind UDP port(%s:%d) success.", bind_ip.c_str(), port);
  }
}

/**
 * 监听UDP服务端口.
 * @param pcthrd 核心类
 */
void CoreThread::RecvUdpData(CoreThread *self) {
  struct sockaddr_in addr;
  socklen_t len;
  char buf[MAX_UDPLEN];
  ssize_t size;

  while (self->started) {
    len = sizeof(addr);
    if ((size = recvfrom(self->udpSock, buf, MAX_UDPLEN, 0,
                         (struct sockaddr *)&addr, &len)) == -1)
      continue;
    if (size != MAX_UDPLEN) buf[size] = '\0';
    UdpData::UdpDataEntry(*self, addr.sin_addr.s_addr, addr.sin_port, buf, size);
  }
}

/**
 * 监听TCP服务端口.
 * @param pcthrd 核心类
 */
void CoreThread::RecvTcpData(CoreThread *pcthrd) {
  int subsock;

  listen(pcthrd->tcpSock, 5);
  while (pcthrd->started) {
    if ((subsock = accept(pcthrd->tcpSock, NULL, NULL)) == -1) continue;
    thread([](CoreThread* coreThread, int subsock){TcpData::TcpDataEntry(coreThread, subsock);}, pcthrd, subsock)
      .detach();
  }
}

void CoreThread::stop() {
  if(!started) {
    throw "CoreThread not started, or already stopped";
  }
  started = false;
  ClearSublayer();
}

void CoreThread::ClearSublayer() {
  /**
   * @note 必须在发送下线信息之后才能关闭套接口.
   */
  for(auto palInfo: pImpl->pallist) {
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
void CoreThread::SendNotifyToAll(CoreThread *pcthrd) {
  Command cmd(*pcthrd);
  if(!pcthrd->pImpl->debugDontBroadcast) {
    cmd.BroadCast(pcthrd->udpSock);
  }
  cmd.DialUp(pcthrd->udpSock);
}

/**
 * 黑名单链表中是否包含此项.
 * @param ipv4 ipv4
 * @return 是否包含
 */
bool CoreThread::BlacklistContainItem(in_addr_t ipv4) const {
  return g_slist_find(pImpl->blacklist, GUINT_TO_POINTER(ipv4));
}

bool CoreThread::IsBlocked(in_addr_t ipv4) const {
  return programData->IsUsingBlacklist() and BlacklistContainItem(ipv4);
}

void CoreThread::Lock() { pthread_mutex_lock(&mutex); }

void CoreThread::Unlock() { pthread_mutex_unlock(&mutex); }

/**
 * 获取好友链表.
 * @return 好友链表
 */
const vector<shared_ptr<PalInfo>>&
CoreThread::GetPalList() { return pImpl->pallist; }


/**
 * 从好友链表中移除所有好友数据(非UI线程安全).
 * @note 鉴于好友链表成员不能被删除，所以将成员改为下线标记即可
 */
void CoreThread::ClearAllPalFromList() {
  /* 清除所有好友的在线标志 */
  for(auto palInfo: pImpl->pallist) {
    palInfo->setOnline(false);
  }
}

/**
 * 从好友链表中获取指定的好友信息数据.
 * @param ipv4 ipv4
 * @return 好友信息数据
 */
const PalInfo* CoreThread::GetPalFromList(PalKey palKey) const {
  for(auto palInfo: pImpl->pallist) {
    if(palInfo->ipv4 == palKey.GetIpv4()) {
      return palInfo.get();
    }
  }
  return nullptr;
}

shared_ptr<PalInfo> CoreThread::GetPal(PalKey palKey) {
  for(auto palInfo: pImpl->pallist) {
    if(palInfo->ipv4 == palKey.GetIpv4()) {
      return palInfo;
    }
  }
  return {};
}

shared_ptr<PalInfo> CoreThread::GetPal(const string& ipv4) {
  return GetPal(PalKey(stringToInAddr(ipv4)));
}

PalInfo* CoreThread::GetPalFromList(PalKey palKey) {
  for(auto palInfo: pImpl->pallist) {
    if(palInfo->ipv4 == palKey.GetIpv4()) {
      return palInfo.get();
    }
  }
  return nullptr;
}

/**
 * 从好友链表中删除指定的好友信息数据(非UI线程安全).
 * @param ipv4 ipv4
 * @note 鉴于好友链表成员不能被删除，所以将成员改为下线标记即可；
 * 鉴于群组中只能包含在线的好友，所以若某群组中包含了此好友，则必须从此群组中删除此好友
 */
void CoreThread::DelPalFromList(PalKey palKey) {
  PalInfo *pal;

  /* 获取好友信息数据，并将其置为下线状态 */
  if (!(pal = GetPalFromList(palKey))) return;
  pal->setOnline(false);
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
  PalInfo *pal;
  /* 如果好友链表中不存在此好友，则视为程序设计出错 */
  if (!(pal = GetPalFromList(palKey))) {
    return;
  }
  pal->setOnline(true);
}

/**
 * 将好友信息数据加入到好友链表(非UI线程安全).
 * @param pal class PalInfo
 * @note 鉴于在线的好友必须被分配到它所属的群组，所以加入好友到好友链表的同时
 * 也应该分配好友到相应的群组
 */
void CoreThread::AttachPalToList(PalInfo *pal) {
  pImpl->pallist.push_back(shared_ptr<PalInfo>(pal));
  pal->setOnline(true);
}

void CoreThread::AttachPalToList(shared_ptr<PalInfo> pal) {
  pImpl->pallist.push_back(pal);
  pal->setOnline(true);
}

void CoreThread::registerCallback(const EventCallback &callback) {
  Lock();
  callbacks.push_back(callback);
  Unlock();
}

void CoreThread::emitNewPalOnline(PPalInfo palInfo) {
  emitEvent(make_shared<NewPalOnlineEvent>(palInfo));
}

void CoreThread::emitNewPalOnline(const PalKey& palKey) {
  auto palInfo = GetPal(palKey);
  if(palInfo) {
    NewPalOnlineEvent event(palInfo);
    emitEvent(make_shared<NewPalOnlineEvent>(palInfo));
  } else {
    LOG_ERROR("emitNewPalOnline meet a unknown key: %s", palKey.ToString().c_str());
  }
}

void CoreThread::emitEvent(shared_ptr<const Event> event) {
  Lock();
  for(EventCallback& callback: callbacks) {
    callback(event);
  }
  Unlock();
}

/**
 * 向好友发送iptux特有的数据.
 * @param pal class PalInfo
 */
void CoreThread::sendFeatureData(PPalInfo pal) {
  Command cmd(*this);
  char path[MAX_PATHLEN];
  const gchar *env;
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
      exit(1);
    }
    cmd.SendSublayer(sock, pal, IPTUX_PHOTOPICOPT, path);
    close(sock);
  }
}

void CoreThread::SendMyIcon(PPalInfo pal, istream& iss) {
    Command(*this).SendMyIcon(udpSock, pal, iss);
}

void CoreThread::AddBlockIp(in_addr_t ipv4) {
  pImpl->blacklist = g_slist_append(pImpl->blacklist, GUINT_TO_POINTER(ipv4));
}

bool CoreThread::SendMessage(PPalInfo palInfo, const string& message) {
  Command cmd(*this);
  cmd.SendMessage(getUdpSock(), palInfo, message.c_str());
  return true;
}

bool CoreThread::SendMessage(PPalInfo pal, const ChipData& chipData) {
  auto ptr = chipData.data.c_str();
  switch (chipData.type) {
    case MessageContentType::STRING:
      /* 文本类型 */
      return SendMessage(pal, chipData.data);
    case MESSAGE_CONTENT_TYPE_PICTURE:
      /* 图片类型 */
      int sock;
      if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        LOG_ERROR(_("Fatal Error!!\nFailed to create new socket!\n%s"), strerror(errno));
        return false;
      }
      Command(*this).SendSublayer(sock, pal, IPTUX_MSGPICOPT, ptr);
      close(sock);  //关闭网络套接口
      /*/* 删除此图片 */
      if(chipData.GetDeleteFileAfterSent()) {
        unlink(ptr);  //此文件已无用处
      }
      return true;
    default:
      g_assert_not_reached();
  }
}

bool CoreThread::SendMsgPara(const MsgPara& para) {
  for(int i = 0; i < int(para.dtlist.size()); ++i) {
    if(!SendMessage(para.pal, para.dtlist[i])) {
      LOG_ERROR("send message failed: %s", para.dtlist[i].ToString().c_str());
      return false;
    }
  }
  return true;
}

void CoreThread::AsyncSendMsgPara(MsgPara&& para) {
  thread t(&CoreThread::SendMsgPara, this, para);
  t.detach();
}

void CoreThread::InsertMessage(const MsgPara& para) {
  MsgPara para2 = para;
  this->emitEvent(make_shared<NewMessageEvent>(move(para2)));
}

void CoreThread::InsertMessage(MsgPara&& para) {
  this->emitEvent(make_shared<NewMessageEvent>(move(para)));
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
  for(auto pal: pImpl->pallist) {
    if (pal->isOnline()) {
      cmd.SendAbsence(udpSock, pal);
    }
    if (pal->isOnline() and pal->isCompatible()) {
      thread t1(bind(&CoreThread::sendFeatureData, this, _1), pal);
      t1.detach();
    }
  }
  Unlock();
}

/**
 * 发送通告本计算机下线的信息.
 * @param pal class PalInfo
 */
void CoreThread::SendBroadcastExit(PPalInfo pal) {
  Command cmd(*this);
  cmd.SendExit(udpSock, pal);
}

int
CoreThread::GetOnlineCount() const {
  int res = 0;
  for(auto pal: pImpl->pallist) {
    if(pal->isOnline()) {
      res++;
    }
  }
  return res;
}

void CoreThread::SendDetectPacket(const string& ipv4) {
  Command(*this).SendDetectPacket(udpSock, stringToInAddr(ipv4));
}

void CoreThread::emitSomeoneExit(const PalKey& palKey) {
  if(!GetPal(palKey)) {
    return;
  }
  DelPalFromList(palKey.GetIpv4());
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
  CHECK(file);
  CHECK(file->fileid >= MAX_SHAREDFILE);
  CHECK(pImpl->privateFiles.count(file->fileid) == 0);
  pImpl->privateFiles[file->fileid] = file;
}

bool CoreThread::DelPrivateFile(uint32_t id) {
  return pImpl->privateFiles.erase(id) >= 1;
}

PFileInfo CoreThread::GetPrivateFileById(uint32_t id) {
  if(id < MAX_SHAREDFILE) {
    FileInfo* f = programData->GetShareFileInfo(id);
    return make_shared<FileInfo>(*f);
  }

  auto res = pImpl->privateFiles.find(id);
  if(res == pImpl->privateFiles.end()) {
    return PFileInfo();
  }
  return res->second;
}

PFileInfo CoreThread::GetPrivateFileByPacketN(uint32_t packageNum, uint32_t filectime) {
  for(auto& i: pImpl->privateFiles) {
    if(i.second->packetn == packageNum && i.second->filenum == filectime) {
      return i.second;
    }
  }
  return PFileInfo();
}

void CoreThread::RegisterTransTask(std::shared_ptr<TransAbstract> task) {
  pImpl->lastTransTaskId++;
  pImpl->transTasks[pImpl->lastTransTaskId] = task;
}

bool CoreThread::TerminateTransTask(int taskId) {
  auto task = pImpl->transTasks.find(taskId);
  if(task == pImpl->transTasks.end()) {
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

std::unique_ptr<TransFileModel>
CoreThread::GetTransTaskStat(int taskId) {
  auto task = pImpl->transTasks.find(taskId);
  if(task == pImpl->transTasks.end()) {
    return {};
  }
  return make_unique<TransFileModel>(task->second->getTransFileModel());
}


}
