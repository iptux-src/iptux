#include "config.h"
#include "CoreThread.h"

#include <gio/gio.h>
#include <glib/gi18n.h>

#include <thread>

#include "ipmsg.h"
#include "support.h"
#include "output.h"
#include "utils.h"
#include "UdpData.h"
#include "TcpData.h"
#include "Command.h"
#include "deplib.h"

using namespace std;

namespace iptux {

CoreThread::CoreThread(shared_ptr<ProgramData> data)
    : programData(data),
      config(data->getConfig()),
      tcpSock(-1),
      udpSock(-1),
      blacklist(nullptr),
      pallist(nullptr),
      started(false)
{
  pthread_mutex_init(&mutex, NULL);
  if(config->GetBool("debug_dont_broadcast")) {
    debugDontBroadcast = true;
  }
}

CoreThread::~CoreThread() {
  if(started) {
    stop();
  }
  g_slist_free(blacklist);
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

  pthread_t pid;

  /* 开启UDP监听服务 */
  pthread_create(&pid, NULL, ThreadFunc(RecvUdpData), this);
  pthread_detach(pid);
  /* 开启TCP监听服务 */
  pthread_create(&pid, NULL, ThreadFunc(RecvTcpData), this);
  pthread_detach(pid);
  /* 通知所有计算机本大爷上线啦 */
  pthread_create(&notifyToAllThread, NULL, ThreadFunc(SendNotifyToAll), this);
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
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (::bind(tcpSock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    int ec = errno;
    close(tcpSock);
    close(udpSock);
    const char* errmsg = g_strdup_printf(_("Fatal Error!! Failed to bind the TCP port(%d)!\n%s"),
                                         port, strerror(ec));
    LOG_WARN("%s", errmsg);
    throw BindFailedException(ec, errmsg);
  }
  if(::bind(udpSock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    int ec = errno;
    close(tcpSock);
    close(udpSock);
    const char* errmsg = g_strdup_printf(_("Fatal Error!! Failed to bind the UDP port(%d)!\n%s"),
                                         port, strerror(ec));
    LOG_WARN("%s", errmsg);
    throw BindFailedException(ec, errmsg);
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
  pthread_t pid;
  int subsock;

  listen(pcthrd->tcpSock, 5);
  while (pcthrd->started) {
    if ((subsock = accept(pcthrd->tcpSock, NULL, NULL)) == -1) continue;
    pthread_create(&pid, NULL, ThreadFunc(TcpData::TcpDataEntry),
                   GINT_TO_POINTER(subsock));
    pthread_detach(pid);
  }
}

void CoreThread::stop() {
  if(!started) {
    throw "CoreThread not started, or already stopped";
  }
  started = false;
  pthread_join(notifyToAllThread, nullptr);
  ClearSublayer();
}

void CoreThread::ClearSublayer() {
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
  if(!pcthrd->debugDontBroadcast) {
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
  return g_slist_find(blacklist, GUINT_TO_POINTER(ipv4));
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
GSList *CoreThread::GetPalList() { return pallist; }


/**
 * 从好友链表中移除所有好友数据(非UI线程安全).
 * @note 鉴于好友链表成员不能被删除，所以将成员改为下线标记即可
 */
void CoreThread::ClearAllPalFromList() {
  PalInfo *pal;
  GSList *tlist;

  /* 清除所有好友的在线标志 */
  tlist = pallist;
  while (tlist) {
    pal = (PalInfo *)tlist->data;
    pal->setOnline(false);
    tlist = g_slist_next(tlist);
  }
}

/**
 * 从好友链表中获取指定的好友信息数据.
 * @param ipv4 ipv4
 * @return 好友信息数据
 */
const PalInfo *CoreThread::GetPalFromList(PalKey palKey) const {
  GSList *tlist;

  tlist = pallist;
  while (tlist) {
    if (((PalInfo *)tlist->data)->ipv4 == palKey.GetIpv4()) break;
    tlist = g_slist_next(tlist);
  }

  return (PalInfo *)(tlist ? tlist->data : NULL);
}

PalInfo *CoreThread::GetPalFromList(PalKey palKey) {
  GSList *tlist;

  tlist = pallist;
  while (tlist) {
    if (((PalInfo *)tlist->data)->ipv4 == palKey.GetIpv4()) break;
    tlist = g_slist_next(tlist);
  }

  return (PalInfo *)(tlist ? tlist->data : NULL);
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
  /* 将好友加入到好友链表 */
  pallist = g_slist_append(pallist, pal);
  pal->setOnline(true);
}

void CoreThread::registerCallback(const EventCallback &callback) {
  Lock();
  callbacks.push_back(callback);
  Unlock();
}

void CoreThread::emitNewPalOnline(PalInfo* palInfo) {
  NewPalOnlineEvent event(palInfo);
  emitEvent(event);
}

void CoreThread::emitEvent(const Event& event) {
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
void CoreThread::sendFeatureData(PalInfo *pal) {
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
    cmd.SendMyIcon(udpSock, pal);
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

void CoreThread::AddBlockIp(in_addr_t ipv4) {
  blacklist = g_slist_append(blacklist, GUINT_TO_POINTER(ipv4));
}

bool CoreThread::SendMessage(PalInfo& palInfo, const string& message) {
  Command cmd(*this);
  cmd.SendMessage(getUdpSock(), &palInfo, message.c_str());
  return true;
}

bool CoreThread::SendMessage(PalInfo& pal, const ChipData& chipData) {
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
      Command(*this).SendSublayer(sock, &pal, IPTUX_MSGPICOPT, ptr);
      close(sock);  //关闭网络套接口
      /*/* 删除此图片 */
      unlink(ptr);  //此文件已无用处
      return true;
    default:
      g_assert_not_reached();
  }
}

bool CoreThread::SendMsgPara(const MsgPara& para) {
  for(int i = 0; i < int(para.dtlist.size()); ++i) {
    if(!SendMessage(*(para.pal), para.dtlist[i])) {
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
  NewMessageEvent event(move(para2));
  this->emitEvent(event);
}

void CoreThread::InsertMessage(MsgPara&& para) {
  NewMessageEvent event(move(para));
  this->emitEvent(event);
}

bool CoreThread::SendAskShared(PalInfo& pal) {
  Command(*this).SendAskShared(getUdpSock(), &pal, 0, NULL);
  return true;
}


}
