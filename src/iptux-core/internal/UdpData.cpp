//
// C++ Implementation: UdpData
//
// Description:
//
//
// Author: cwll <cwll2009@126.com> ,(C)2012
//        Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
// 2012.02 在接收消息中去掉了群组模式，群组模式只用来发消息
//
#include "config.h"
#include "UdpData.h"

#include <cinttypes>
#include <functional>
#include <thread>

#include <fcntl.h>
#include <unistd.h>

#include <glib/gi18n.h>

#include "iptux-core/internal/Command.h"
#include "iptux-core/internal/CommandMode.h"
#include "iptux-core/internal/RecvFile.h"
#include "iptux-core/internal/SendFile.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"

#define NULL_OBJECT 0x02

using namespace std;
using namespace std::placeholders;

namespace iptux {

/**
 * 类构造函数.
 */
UdpData::UdpData(CoreThread& coreThread)
    : coreThread(coreThread), ipv4({0}), size(0), encode(NULL) {}

/**
 * 类析构函数.
 */
UdpData::~UdpData() {
  g_free(encode);
}

/**
 * UDP数据解析入口.
 * @param ipv4 ipv4
 * @param buf[] 数据缓冲区
 * @param size 数据有效长度
 */
unique_ptr<UdpData> UdpData::UdpDataEntry(CoreThread& coreThread,
                                          in_addr ipv4,
                                          int port,
                                          const char buf[],
                                          size_t size) {
  return UdpDataEntry(coreThread, ipv4, port, buf, size, true);
}

unique_ptr<UdpData> UdpData::UdpDataEntry(CoreThread& coreThread,
                                          in_addr ipv4,
                                          int port,
                                          const char buf[],
                                          size_t size,
                                          bool run) {
  if (Log::IsDebugEnabled()) {
    LOG_DEBUG("received udp message from %s:%d, size %zu\n%s",
              inAddrToString(ipv4).c_str(), port, size,
              stringDumpAsCString(string(buf, size)).c_str());
  } else {
    LOG_INFO("received udp message from %s:%d, size %zu",
             inAddrToString(ipv4).c_str(), port, size);
  }
  auto udata = make_unique<UdpData>(coreThread);

  udata->ipv4 = ipv4;
  udata->size = size < MAX_UDPLEN ? size : MAX_UDPLEN;
  memcpy(udata->buf, buf, size);
  if (size != MAX_UDPLEN)
    udata->buf[size] = '\0';
  if (run) {
    udata->DispatchUdpData();
  }
  return udata;
}

/**
 * 分派UDP数据到相应的函数去进行处理.
 */
void UdpData::DispatchUdpData() {
  uint32_t commandno;

  /* 如果开启了黑名单处理功能，且此地址正好被列入了黑名单 */
  /* 嘿嘿，那就不要怪偶心狠手辣了 */
  if (coreThread.IsBlocked(ipv4)) {
    LOG_INFO("address is blocked: %s", inAddrToString(ipv4).c_str());
    return;
  }

  /* 决定消息去向 */
  commandno = iptux_get_dec_number(buf, ':', 4);
  auto commandMode = GET_MODE(commandno);
  LOG_INFO("command NO.: [0x%x] %s", commandno,
           CommandMode(commandMode).toString().c_str());
  switch (commandMode) {
    case IPMSG_BR_ENTRY:
      SomeoneEntry();
      break;
    case IPMSG_BR_EXIT:
      SomeoneExit();
      break;
    case IPMSG_ANSENTRY:
      SomeoneAnsEntry();
      break;
    case IPMSG_BR_ABSENCE:
      SomeoneAbsence();
      break;
    case IPMSG_SENDMSG:
      SomeoneSendmsg();
      break;
    case IPMSG_RECVMSG:
      SomeoneRecvmsg();
      break;
    case IPTUX_ASKSHARED:
      SomeoneAskShared();
      break;
    case IPTUX_SENDICON:
      SomeoneSendIcon();
      break;
    case IPTUX_SEND_SIGN:
      SomeoneSendSign();
      break;
    case IPTUX_SENDMSG:
      SomeoneBcstmsg();
      break;
    default:
      LOG_WARN("unknown command mode: 0x%lx", commandMode);
      break;
  }
}

/**
 * 好友信息数据丢失默认处理函数.
 * 若xx并不在好友列表中，但是程序却期望能够接受xx的本次会话，
 * 所以必须以默认数据构建好友信息数据并插入好友列表 \n
 */
void UdpData::SomeoneLost() {
  PalInfo* pal;

  auto g_progdt = coreThread.getProgramData();

  /* 创建好友数据 */
  pal = new PalInfo;
  pal->ipv4 = ipv4;
  pal->segdes = g_strdup(g_progdt->FindNetSegDescription(ipv4).c_str());
  auto version = iptux_get_section_string(buf, ':', 0);
  auto user = iptux_get_section_string(buf, ':', 2);
  auto host = iptux_get_section_string(buf, ':', 3);
  (*pal)
      .setVersion(version ? version : "?")
      .setUser(user ? user : "???")
      .setHost(host ? host : "???")
      .setEncode(encode ? encode : "utf-8")
      .setName(_("mysterious"))
      .setGroup(_("mysterious"));
  pal->photo = NULL;
  pal->sign = NULL;
  pal->iconfile = g_strdup(g_progdt->palicon);
  pal->setOnline(true);
  pal->packetn = 0;
  pal->rpacketn = 0;

  /* 加入好友列表 */
  coreThread.Lock();
  coreThread.AttachPalToList(PPalInfo(pal));
  coreThread.Unlock();
  // coreThread.AttachItemToPaltree(ipv4);
}

/**
 * 好友上线.
 */
void UdpData::SomeoneEntry() {
  using namespace std::placeholders;

  Command cmd(coreThread);
  shared_ptr<PalInfo> pal;

  auto programData = coreThread.getProgramData();
  /* 转换缓冲区数据编码 */
  ConvertEncode(programData->encode);

  /* 加入或更新好友列表 */
  coreThread.Lock();
  if ((pal = coreThread.GetPal(ipv4))) {
    UpdatePalInfo(pal.get());
    coreThread.UpdatePalToList(ipv4);
  } else {
    pal = CreatePalInfo();
    coreThread.AttachPalToList(pal);
  }
  coreThread.Unlock();
  coreThread.emitNewPalOnline(pal);

  /* 通知好友本大爷在线 */
  cmd.SendAnsentry(coreThread.getUdpSock(), pal);
  if (pal->isCompatible()) {
    thread t1(bind(&CoreThread::sendFeatureData, &coreThread, _1), pal);
    t1.detach();
  }
}

/**
 * 好友退出.
 */
void UdpData::SomeoneExit() {
  coreThread.emitSomeoneExit(PalKey(ipv4));
}

/**
 * 好友在线.
 */
void UdpData::SomeoneAnsEntry() {
  Command cmd(coreThread);
  const char* ptr;

  auto g_progdt = coreThread.getProgramData();

  /* 若好友不兼容iptux协议，则需转码 */
  ptr = iptux_skip_string(buf, size, 3);
  if (!ptr || *ptr == '\0')
    ConvertEncode(g_progdt->encode);

  /* 加入或更新好友列表 */
  coreThread.Lock();
  shared_ptr<PalInfo> pal;
  if ((pal = coreThread.GetPal(ipv4))) {
    UpdatePalInfo(pal.get());
    coreThread.UpdatePalToList(ipv4);
  } else {
    pal = CreatePalInfo();
    coreThread.AttachPalToList(pal);
  }
  coreThread.Unlock();
  coreThread.emitNewPalOnline(pal);

  /* 更新本大爷的数据信息 */
  if (pal->isCompatible()) {
    thread t1(bind(&CoreThread::sendFeatureData, &coreThread, _1), pal);
    t1.detach();
  } else if (strcasecmp(g_progdt->encode.c_str(), pal->getEncode().c_str()) !=
             0) {
    cmd.SendAnsentry(coreThread.getUdpSock(), pal);
  }
}

/**
 * 好友更改个人信息.
 */
void UdpData::SomeoneAbsence() {
  PPalInfo pal;
  const char* ptr;

  auto g_progdt = coreThread.getProgramData();

  /* 若好友不兼容iptux协议，则需转码 */
  pal = coreThread.GetPal(ipv4);  //利用好友链表只增不减的特性，无须加锁
  ptr = iptux_skip_string(buf, size, 3);
  if (!ptr || *ptr == '\0') {
    if (pal) {
      string s(pal->getEncode());
      ConvertEncode(s);
    } else {
      ConvertEncode(g_progdt->encode);
    }
  }

  /* 加入或更新好友列表 */
  coreThread.Lock();
  if (pal) {
    UpdatePalInfo(pal.get());
    coreThread.UpdatePalToList(ipv4);
  } else {
    coreThread.AttachPalToList(CreatePalInfo());
  }
  coreThread.Unlock();
}

/**
 * 好友发送消息.
 *
 */
void UdpData::SomeoneSendmsg() {
  Command cmd(coreThread);
  uint32_t commandno, packetno;
  char* text;

  auto g_progdt = coreThread.getProgramData();

  /* 如果对方兼容iptux协议，则无须再转换编码 */
  auto pal = coreThread.GetPal(ipv4);
  if (!pal || !pal->isCompatible()) {
    if (pal) {
      ConvertEncode(pal->getEncode());
    } else {
      ConvertEncode(g_progdt->encode);
    }
  }
  /* 确保好友在线，并对编码作出适当调整 */
  pal = AssertPalOnline();
  if (strcasecmp(pal->getEncode().c_str(), encode ? encode : "utf-8") != 0) {
    pal->setEncode(encode ? encode : "utf-8");
  }

  /* 回复好友并检查此消息是否过时 */
  commandno = iptux_get_dec_number(buf, ':', 4);
  packetno = iptux_get_dec_number(buf, ':', 1);
  if (commandno & IPMSG_SENDCHECKOPT) {
    cmd.SendReply(coreThread.getUdpSock(), pal->GetKey(), packetno);
  }
  if (packetno <= pal->packetn)
    return;
  pal->packetn = packetno;

  /* 插入消息&在消息队列中注册 */
  text = ipmsg_get_attach(buf, ':', 5);
  if (text && *text != '\0') {
    InsertMessage(pal, GROUP_BELONG_TYPE_REGULAR, text);
  }
  g_free(text);

  /* 标记位处理 先处理底层数据，后面显示窗口*/
  if (commandno & IPMSG_FILEATTACHOPT) {
    if ((commandno & IPTUX_SHAREDOPT) && (commandno & IPTUX_PASSWDOPT)) {
      coreThread.emitEvent(make_shared<PasswordRequiredEvent>(pal->GetKey()));
    } else {
      RecvPalFile();
    }
  }

  // if (grpinf->dialog) {
  //   auto window = GTK_WIDGET(grpinf->dialog);
  //   auto dlgpr = (DialogPeer *)(g_object_get_data(G_OBJECT(window),
  //   "dialog")); dlgpr->ShowDialogPeer(dlgpr);
  // }
}

/**
 * 好友接收到消息.
 */
void UdpData::SomeoneRecvmsg() {
  uint32_t packetno;
  PPalInfo pal;

  if ((pal = coreThread.GetPal(ipv4))) {
    packetno = iptux_get_dec_number(buf, ':', 5);
    if (packetno == pal->rpacketn)
      pal->rpacketn = 0;  //标记此包编号已经被回复
  } else {
    LOG_WARN("message from unknown pal: %s", inAddrToString(ipv4).c_str());
  }
}

/**
 * 好友请求本计算机的共享文件.
 */
void UdpData::SomeoneAskShared() {
  Command cmd(coreThread);
  PPalInfo pal;
  char* passwd;

  if (!(pal = coreThread.GetPal(ipv4)))
    return;

  auto limit = coreThread.GetAccessPublicLimit();
  if (limit.empty()) {
    thread([](CoreThread* coreThread,
              PPalInfo pal) { ThreadAskSharedFile(coreThread, pal); },
           &coreThread, pal)
        .detach();
  } else if (!(iptux_get_dec_number(buf, ':', 4) & IPTUX_PASSWDOPT)) {
    cmd.SendFileInfo(coreThread.getUdpSock(), pal->GetKey(),
                     IPTUX_SHAREDOPT | IPTUX_PASSWDOPT, "");
  } else if ((passwd = ipmsg_get_attach(buf, ':', 5))) {
    if (limit == passwd) {
      thread([](CoreThread* coreThread,
                PPalInfo pal) { ThreadAskSharedFile(coreThread, pal); },
             &coreThread, pal)
          .detach();
    }
    g_free(passwd);
  }
}

/**
 * 好友发送头像数据.
 */
void UdpData::SomeoneSendIcon() {
  PPalInfo pal;
  char* iconfile;

  if (!(pal = coreThread.GetPal(ipv4)) || pal->isChanged()) {
    return;
  }

  /* 接收并更新数据 */
  if ((iconfile = RecvPalIcon())) {
    g_free(pal->iconfile);
    pal->iconfile = iconfile;
    coreThread.EmitIconUpdate(PalKey(ipv4));
  }
}

/**
 * 好友发送个性签名.
 */
void UdpData::SomeoneSendSign() {
  PPalInfo pal;
  char* sign;

  if (!(pal = coreThread.GetPal(ipv4)))
    return;

  /* 若好友不兼容iptux协议，则需转码 */
  if (!pal->isCompatible()) {
    ConvertEncode(pal->getEncode());
  }
  /* 对编码作适当调整 */
  if (strcasecmp(pal->getEncode().c_str(), encode ? encode : "utf-8") != 0) {
    pal->setEncode(encode ? encode : "utf-8");
  }
  /* 更新 */
  if ((sign = ipmsg_get_attach(buf, ':', 5))) {
    g_free(pal->sign);
    pal->sign = sign;
    coreThread.Lock();
    coreThread.UpdatePalToList(ipv4);
    coreThread.Unlock();
    coreThread.emitNewPalOnline(pal->GetKey());
  }
}

/**
 * 好友广播消息.
 */
void UdpData::SomeoneBcstmsg() {
  uint32_t commandno, packetno;
  char* text;

  auto g_progdt = coreThread.getProgramData();

  /* 如果对方兼容iptux协议，则无须再转换编码 */
  auto pal = coreThread.GetPal(ipv4);
  if (!pal || !pal->isCompatible()) {
    if (pal) {
      ConvertEncode(pal->getEncode());
    } else {
      ConvertEncode(g_progdt->encode);
    }
  }
  /* 确保好友在线，并对编码作出适当调整 */
  pal = AssertPalOnline();
  if (strcasecmp(pal->getEncode().c_str(), encode ? encode : "utf-8") != 0) {
    pal->setEncode(encode ? encode : "utf-8");
  }

  /* 检查此消息是否过时 */
  packetno = iptux_get_dec_number(buf, ':', 1);
  if (packetno <= pal->packetn) {
    return;
  }
  pal->packetn = packetno;

  /* 插入消息&在消息队列中注册 */
  text = ipmsg_get_attach(buf, ':', 5);
  if (text && *text != '\0') {
    commandno = iptux_get_dec_number(buf, ':', 4);
    /* 插入消息 */
    switch (GET_OPT(commandno)) {
      case IPTUX_BROADCASTOPT:
        InsertMessage(pal, GROUP_BELONG_TYPE_BROADCAST, text);
        break;
      case IPTUX_GROUPOPT:
        InsertMessage(pal, GROUP_BELONG_TYPE_GROUP, text);
        break;
      case IPTUX_SEGMENTOPT:
        InsertMessage(pal, GROUP_BELONG_TYPE_SEGMENT, text);
        break;
      case IPTUX_REGULAROPT:
      default:
        InsertMessage(pal, GROUP_BELONG_TYPE_REGULAR, text);
        break;
    }
  }
  g_free(text);
}

/**
 * 创建好友信息数据.
 * @return 好友数据
 */
shared_ptr<PalInfo> UdpData::CreatePalInfo() {
  auto programData = coreThread.getProgramData();
  auto pal = make_shared<PalInfo>();
  pal->ipv4 = ipv4;
  pal->segdes = g_strdup(programData->FindNetSegDescription(ipv4).c_str());
  auto version = iptux_get_section_string(buf, ':', 0);
  auto user = iptux_get_section_string(buf, ':', 2);
  auto host = iptux_get_section_string(buf, ':', 3);
  (*pal)
      .setVersion(version ? version : "?")
      .setUser(user ? user : "???")
      .setHost(host ? host : "???");
  auto name = ipmsg_get_attach(buf, ':', 5);
  if (!name) {
    pal->setName(_("mysterious"));
  } else {
    pal->setName(name);
  }
  pal->setGroup(GetPalGroup());
  pal->photo = NULL;
  pal->sign = NULL;
  if (!(pal->iconfile = GetPalIcon()))
    pal->iconfile = g_strdup(programData->palicon);
  auto localEncode = GetPalEncode();
  if (localEncode) {
    pal->setEncode(localEncode);
    pal->setCompatible(true);
  } else {
    pal->setEncode(encode ? encode : "utf-8");
  }
  pal->setOnline(true);
  pal->packetn = 0;
  pal->rpacketn = 0;

  return pal;
}

/**
 * 更新好友信息数据.
 * @param pal 好友数据
 */
void UdpData::UpdatePalInfo(PalInfo* pal) {
  auto g_progdt = coreThread.getProgramData();

  g_free(pal->segdes);
  pal->segdes = g_strdup(g_progdt->FindNetSegDescription(ipv4).c_str());
  auto version = iptux_get_section_string(buf, ':', 0);
  auto user = iptux_get_section_string(buf, ':', 2);
  auto host = iptux_get_section_string(buf, ':', 3);
  (*pal)
      .setVersion(version ? version : "?")
      .setUser(user ? user : "???")
      .setHost(host ? host : "???");
  if (!pal->isChanged()) {
    auto name = ipmsg_get_attach(buf, ':', 5);
    if (!name) {
      pal->setName(_("mysterious"));
    } else {
      pal->setName(name);
    }
    pal->setGroup(GetPalGroup());
    g_free(pal->iconfile);
    if (!(pal->iconfile = GetPalIcon()))
      pal->iconfile = g_strdup(g_progdt->palicon);
    pal->setCompatible(false);
    auto localEncode = GetPalEncode();
    if (localEncode) {
      pal->setEncode(localEncode);
      pal->setCompatible(true);
    } else {
      pal->setEncode(encode ? encode : "utf-8");
    }
  }
  pal->setOnline(true);
  pal->packetn = 0;
  pal->rpacketn = 0;
}

/**
 * 插入消息.
 * @param pal class PalInfo
 * @param btype 消息所属类型
 * @param msg 消息
 */
void UdpData::InsertMessage(PPalInfo pal,
                            GroupBelongType btype,
                            const char* msg) {
  MsgPara para(coreThread.GetPal(pal->GetKey()));

  /* 构建消息封装包 */
  para.stype = MessageSourceType::PAL;
  para.btype = btype;
  ChipData chip(MESSAGE_CONTENT_TYPE_STRING, msg);
  para.dtlist.push_back(move(chip));

  /* 交给某人处理吧 */
  coreThread.InsertMessage(move(para));
}

void UdpData::ConvertEncode(const char* enc) {
  string encode(enc);
  ConvertEncode(encode);
}

/**
 * 将缓冲区中的数据转换为utf8编码.
 * @param enc 原数据首选编码
 */
void UdpData::ConvertEncode(const string& enc) {
  char* ptr;
  size_t len;

  /* 将缓冲区内有效的'\0'字符转换为ASCII字符 */
  ptr = buf + strlen(buf) + 1;
  while ((size_t)(ptr - buf) <= size) {
    *(ptr - 1) = NULL_OBJECT;
    ptr += strlen(ptr) + 1;
  }

  /* 转换字符集编码 */
  /**
   * @note 请不要采用以下做法，它在某些环境下将导致致命错误: \n
   * if (g_utf8_validate(buf, -1, NULL)) {encode = g_strdup("utf-8")} \n
   * e.g. 系统编码为GB18030的xx发送来纯ASCII字符串 \n
   */
  if (!enc.empty() && strcasecmp(enc.c_str(), "utf-8") != 0 &&
      (ptr = convert_encode(buf, "utf-8", enc.c_str())))
    encode = g_strdup(enc.c_str());
  else
    ptr = iptux_string_validate(buf, coreThread.getProgramData()->codeset,
                                &encode);
  if (ptr) {
    len = strlen(ptr);
    size = len < MAX_UDPLEN ? len : MAX_UDPLEN;
    memcpy(buf, ptr, size);
    if (size < MAX_UDPLEN)
      buf[size] = '\0';
    g_free(ptr);
  }

  /* 将缓冲区内的ASCII字符还原为'\0'字符 */
  ptr = buf;
  while ((ptr = (char*)memchr(ptr, NULL_OBJECT, buf + size - ptr))) {
    *ptr = '\0';
    ptr++;
  }
}

/**
 * 获取好友群组名称.
 * @return 群组
 */
string UdpData::GetPalGroup() {
  const char* ptr;

  if ((ptr = iptux_skip_string(buf, size, 1)) && *ptr != '\0')
    return ptr;
  return "";
}

/**
 * 获取好友头像图标.
 * @return 头像
 */
char* UdpData::GetPalIcon() {
  char path[MAX_PATHLEN];
  const char* ptr;

  if ((ptr = iptux_skip_string(buf, size, 2)) && *ptr != '\0') {
    snprintf(path, MAX_PATHLEN, __PIXMAPS_PATH "/icon/%s", ptr);
    if (access(path, F_OK) == 0)
      return g_strdup(ptr);
  }
  return NULL;
}

/**
 * 获取好友系统编码.
 * @return 编码
 */
char* UdpData::GetPalEncode() {
  const char* ptr;

  if ((ptr = iptux_skip_string(buf, size, 3)) && *ptr != '\0')
    return g_strdup(ptr);
  return NULL;
}

/**
 * 接收好友头像数据.
 * @return 头像文件名
 */
char* UdpData::RecvPalIcon() {
  char path[MAX_PATHLEN];
  char* iconfile;
  size_t len;
  int fd;

  /* 若无头像数据则返回null */
  if ((len = strlen(buf) + 1) >= size)
    return NULL;

  /* 将头像数据刷入磁盘 */
  snprintf(path, MAX_PATHLEN, "%s" ICON_PATH "/%" PRIx32,
           g_get_user_cache_dir(), inAddrToUint32(ipv4));
  Helper::prepareDir(path);
  if ((fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
    LOG_ERROR("write icon to path failed: %s", path);
    return NULL;
  }
  xwrite(fd, buf + len, size - len);
  close(fd);
  iconfile = g_strdup_printf("%" PRIx32, inAddrToUint32(ipv4));
  return iconfile;
}

/**
 * 确保好友一定在线.
 * @return 好友数据
 */
PPalInfo UdpData::AssertPalOnline() {
  PPalInfo pal;

  if ((pal = coreThread.GetPal(ipv4))) {
    /* 既然好友不在线，那么他自然不在列表中 */
    if (!pal->isOnline()) {
      pal->setOnline(true);
      coreThread.Lock();
      coreThread.UpdatePalToList(ipv4);
      coreThread.Unlock();
      coreThread.emitNewPalOnline(pal->GetKey());
    }
  } else {
    SomeoneLost();
    pal = coreThread.GetPal(ipv4);
  }

  return pal;
}

/**
 * 接收好友文件信息.
 */
void UdpData::RecvPalFile() {
  uint32_t packetno, commandno;
  const char* ptr;

  packetno = iptux_get_dec_number(buf, ':', 1);
  commandno = iptux_get_dec_number(buf, ':', 4);
  ptr = iptux_skip_string(buf, size, 1);
  /* 只有当此为共享文件信息或文件信息不为空才需要接收 */
  if ((commandno & IPTUX_SHAREDOPT) || (ptr && *ptr != '\0')) {
    thread(
        [](CoreThread* coreThread, PPalInfo pal, string data, int packetno) {
          RecvFile::RecvEntry(coreThread, pal, data, packetno);
        },
        &coreThread, coreThread.GetPal(ipv4), ptr, packetno)
        .detach();
  }
}

/**
 * 某好友请求本计算机的共享文件.
 * @param pal class PalInfo
 */
void UdpData::ThreadAskSharedFile(CoreThread* coreThread, PPalInfo pal) {
  auto g_progdt = coreThread->getProgramData();

  if (g_progdt->IsFilterFileShareRequest()) {
    coreThread->emitEvent(make_shared<PermissionRequiredEvent>(pal->GetKey()));
  } else {
    SendFile::SendSharedInfoEntry(coreThread, pal);
  }
}

}  // namespace iptux
