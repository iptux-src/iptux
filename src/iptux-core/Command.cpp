//
// C++ Implementation: Command
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "config.h"
#include "Command.h"

#include <cinttypes>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "iptux-core/deplib.h"
#include "iptux-core/support.h"
#include "iptux-core/utils.h"
#include "iptux-core/output.h"
#include "iptux-core/Exception.h"
#include "iptux-core/TransAbstract.h"

using namespace std;

namespace iptux {

uint32_t Command::packetn = 1;

static
PPalInfo getAndCheckPalInfo(CoreThread& coreThread, const PalKey& palKey) {
  auto res = coreThread.GetPal(palKey);
  if(!res) {
    throw Exception(ErrorCode::PAL_KEY_NOT_EXIST, stringFormat("palkey not exist: %s", palKey.ToString().c_str()));
  }
  return res;
}


/**
 * @brief
 *
 * @param sockfd
 * @param buf
 * @param len
 * @param flags
 * @param dest_addr
 * @param addrlen
 * @return true means succcess
 * @return false means failed
 */
static bool commandSendTo(int sockfd, const void * buf, size_t len, int flags, in_addr_t ipv4, int port) {
  if(Log::IsDebugEnabled()) {
    LOG_DEBUG("send udp message to %s:%d, size %d\n%s", inAddrToString(ipv4).c_str(),
      port, int(len), stringDump(string((const char*)buf, len)).c_str());
  } else if(Log::IsInfoEnabled()) {
    LOG_INFO("send udp message to %s:%d, size %d", inAddrToString(ipv4).c_str(), port, int(len));
  }
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = ipv4;
  return sendto(sockfd, buf, len, flags, (struct sockaddr *)(&addr), sizeof(struct sockaddr_in)) != -1;
}

static bool commandSendTo(int sockfd, const void * buf, size_t len, int flags, in_addr_t ipv4) {
  return commandSendTo(sockfd, buf, len, flags, ipv4, IPTUX_DEFAULT_PORT);
}

/**
 * 类构造函数.
 */
Command::Command(CoreThread& coreThread)
    : coreThread(coreThread),
      size(0),
      buf("") {}

/**
 * 类析构函数.
 */
Command::~Command() {}

/**
 * 向局域网所有计算机广播上线信息.
 * @param sock udp socket
 */
void Command::BroadCast(int sock) {
  GSList *list, *tlist;

  auto programData = coreThread.getProgramData();
  CreateCommand(IPMSG_ABSENCEOPT | IPMSG_BR_ENTRY, programData->nickname.c_str());
  ConvertEncode(programData->encode);
  CreateIptuxExtra(programData->encode);

  tlist = list = get_sys_broadcast_addr(sock);
  while (tlist) {
    commandSendTo(sock, buf, size, 0, GPOINTER_TO_UINT(tlist->data));
    g_usleep(9999);
    tlist = g_slist_next(tlist);
  }
  g_slist_free(list);
}

/**
 * 向局域网某些计算机单独发送上线信息.
 * @param sock udp socket
 */
void Command::DialUp(int sock) {
  in_addr_t startip, endip, ipv4;
  NetSegment *pns;

  auto programData = coreThread.getProgramData();
  CreateCommand(IPMSG_DIALUPOPT | IPMSG_ABSENCEOPT | IPMSG_BR_ENTRY,
                programData->nickname.c_str());
  ConvertEncode(programData->encode);
  CreateIptuxExtra(programData->encode);

  //与某些代码片段的获取网段描述相冲突，必须复制出来使用
  programData->Lock();
  vector<NetSegment> list = programData->copyNetSegments();
  programData->Unlock();
  for(int i = 0; i < int(list.size()); ++i) {
    pns = &list[i];
    inet_pton(AF_INET, pns->startip.c_str(), &startip);
    startip = ntohl(startip);
    inet_pton(AF_INET, pns->endip.c_str(), &endip);
    endip = ntohl(endip);
    ipv4_order(&startip, &endip);
    ipv4 = startip;
    while (ipv4 <= endip) {
      commandSendTo(sock, buf, size, 0, htonl(ipv4));
      g_usleep(999);
      ipv4++;
    }
  }
}

/**
 * 回复好友本人在线.
 * @param sock udp socket
 * @param pal class PalInfo
 */
void Command::SendAnsentry(int sock, CPPalInfo pal) {
  auto programData = coreThread.getProgramData();

  CreateCommand(IPMSG_ABSENCEOPT | IPMSG_ANSENTRY, programData->nickname.c_str());
  ConvertEncode(pal->encode);
  CreateIptuxExtra(pal->encode);
  commandSendTo(sock, buf, size, 0, pal->ipv4);
}

/**
 * 通告好友本人下线.
 * @param sock udp socket
 * @param pal class PalInfo
 */
void Command::SendExit(int sock, CPPalInfo pal) {
  CreateCommand(IPMSG_DIALUPOPT | IPMSG_BR_EXIT, NULL);
  ConvertEncode(pal->encode);
  commandSendTo(sock, buf, size, 0, pal->ipv4);
}

/**
 * 通告好友本人个人信息已变.
 * @param sock udp socket
 * @param pal class PalInfo
 */
void Command::SendAbsence(int sock, CPPalInfo pal) {
  auto programData = coreThread.getProgramData();
  CreateCommand(IPMSG_ABSENCEOPT | IPMSG_BR_ABSENCE,
                programData->nickname.c_str());
  ConvertEncode(pal->encode);
  CreateIptuxExtra(pal->encode);
  commandSendTo(sock, buf, size, 0, pal->ipv4);
}

/**
 * 尝试着给某计算机发送一个上线信息数据包.
 * @param sock udp socket
 * @param ipv4 ipv4 address
 */
void Command::SendDetectPacket(int sock, in_addr_t ipv4) {
  auto programData = coreThread.getProgramData();
  CreateCommand(IPMSG_DIALUPOPT | IPMSG_ABSENCEOPT | IPMSG_BR_ENTRY,
                programData->nickname.c_str());
  ConvertEncode(programData->encode);
  CreateIptuxExtra(programData->encode);
  commandSendTo(sock, buf, size, 0, ipv4);
}

/**
 * 给好友发送消息.
 * @param sock udp socket
 * @param pal class PalInfo
 * @param msg 消息数据
 */
void Command::SendMessage(int sock, CPPalInfo pal, const char *msg) {
  uint32_t packetno;
  uint8_t count;

  auto pal2 = coreThread.GetPal(pal->GetKey());
  if(!pal2) {
    throw Exception(ErrorCode::PAL_KEY_NOT_EXIST);
  }

  pal2->rpacketn = packetno = packetn;  //此数据包需要检验回复
  CreateCommand(IPMSG_SENDCHECKOPT | IPMSG_SENDMSG, msg);
  ConvertEncode(pal->encode);

  count = 0;
  do {
    commandSendTo(sock, buf, size, 0, pal->ipv4);
    g_usleep(1000000);
    count++;
  } while (pal->rpacketn == packetno && count < MAX_RETRYTIMES);
  if (pal->rpacketn == packetno) {
    FeedbackError(pal, GROUP_BELONG_TYPE_REGULAR,
                  _("Your pal didn't receive the packet. He or she is offline maybe."));
  }
}

/**
 * 回复已收到消息.
 * @param sock udp socket
 * @param pal class PalInfo
 * @param packetno 好友消息的包编号
 */
void Command::SendReply(int sock, CPPalInfo pal, uint32_t packetno) {
  char packetstr[11];  // 10 +1 =11

  snprintf(packetstr, 11, "%" PRIu32, packetno);
  CreateCommand(IPMSG_SENDCHECKOPT | IPMSG_RECVMSG, packetstr);
  ConvertEncode(pal->encode);

  commandSendTo(sock, buf, size, 0, pal->ipv4);
}

void Command::SendReply(int sock, const PalKey& palKey, uint32_t packetno) {
  auto palInfo = getAndCheckPalInfo(coreThread, palKey);
  SendReply(sock, palInfo, packetno);
}

/**
 * 群发消息(被其他函数调用).
 * @param sock udp socket
 * @param pal class PalInfo
 * @param msg 消息数据
 */
void Command::SendGroupMsg(int sock, CPPalInfo pal, const char *msg) {
  CreateCommand(IPMSG_BROADCASTOPT | IPMSG_SENDMSG, msg);
  ConvertEncode(pal->encode);
  commandSendTo(sock, buf, size, 0, pal->ipv4);
}

/**
 * 发送群组消息(被其他函数调用).
 * @param sock udp socket
 * @param pal class PalInfo
 * @param opttype 命令额外选项
 * @param msg 消息数据
 */
void Command::SendUnitMsg(int sock, CPPalInfo pal, uint32_t opttype,
                          const char *msg) {
  CreateCommand(opttype | IPTUX_SENDMSG, msg);
  ConvertEncode(pal->encode);
  commandSendTo(sock, buf, size, 0, pal->ipv4);
}

/**
 * 向好友请求文件数据.
 * @param sock tcp socket
 * @param pal class PalInfo
 * @param packetno 好友消息的包编号
 * @param fileid 文件ID标识
 * @param offset 文件偏移量
 * @return 消息发送成功与否
 */
bool Command::SendAskData(int sock, CPPalInfo pal, uint32_t packetno,
                          uint32_t fileid, int64_t offset) {
  char attrstr[35];  // 8+1+8+1+16 +1 =35
  struct sockaddr_in addr;
  const char *iptuxstr = "iptux";

  snprintf(attrstr, 35, "%" PRIx32 ":%" PRIx32 ":%" PRIx64, packetno, fileid,
           offset);
  // IPMSG和Feiq的命令字段都是只有IPMSG_GETFILEDATA,使用(IPMSG_FILEATTACHOPT |
  // IPMSG_GETFILEDATA）
  //会产生一些潜在的不兼容问题,所以在发往非iptux时只使用IPMSG_GETFILEDATA
  if (strstr(pal->version, iptuxstr))
    CreateCommand(IPMSG_FILEATTACHOPT | IPMSG_GETFILEDATA, attrstr);
  else
    CreateCommand(IPMSG_GETFILEDATA, attrstr);
  ConvertEncode(pal->encode);

  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(IPTUX_DEFAULT_PORT);
  addr.sin_addr.s_addr = pal->ipv4;

  if (((connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) &&
       (errno != EINTR)) ||
      (xwrite(sock, buf, size) == -1))
    return false;

  return true;
}

bool Command::SendAskData(int sock, const PalKey& palKey, uint32_t packetno,
                          uint32_t fileid, int64_t offset)
{
  auto palInfo = getAndCheckPalInfo(coreThread, palKey);
  return SendAskData(sock, palInfo, packetno, fileid, offset);
}

/**
 * 向好友请求目录文件.
 * @param sock tcp socket
 * @param pal class PalInfo
 * @param packetno 好友消息的包编号
 * @param fileid 文件ID标识
 * @return 消息发送成功与否
 */
bool Command::SendAskFiles(int sock, const PalKey& palKey, uint32_t packetno,
                           uint32_t fileid)
{
  auto palInfo = getAndCheckPalInfo(coreThread, palKey);
  return SendAskFiles(sock, palInfo, packetno, fileid);
}

bool Command::SendAskFiles(int sock, CPPalInfo pal, uint32_t packetno,
                           uint32_t fileid) {
  char attrstr[20];  // 8+1+8+1+1 +1  =20
  struct sockaddr_in addr;

  snprintf(attrstr, 20, "%" PRIx32 ":%" PRIx32 ":0", packetno,
           fileid);  //兼容LanQQ软件
  CreateCommand(IPMSG_FILEATTACHOPT | IPMSG_GETDIRFILES, attrstr);
  ConvertEncode(pal->encode);

  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(IPTUX_DEFAULT_PORT);
  addr.sin_addr.s_addr = pal->ipv4;

  if (((connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) &&
       (errno != EINTR)) ||
      (xwrite(sock, buf, size) == -1))
    return false;

  return true;
}

/**
 * 向好友请求共享文件信息.
 * @param sock udp socket
 * @param pal class PalInfo
 * @param opttype 命令额外选项
 * @param attach 附加数据，即密码
 */
void Command::SendAskShared(int sock, CPPalInfo pal, uint32_t opttype,
                            const char *attach) {
  CreateCommand(opttype | IPTUX_ASKSHARED, attach);
  ConvertEncode(pal->encode);
  commandSendTo(sock, buf, size, 0, pal->ipv4);
}

void Command::SendAskShared(int sock, const PalKey& palKey, uint32_t opttype,
                            const char *attach)
{
  auto palInfo = getAndCheckPalInfo(coreThread, palKey);
  SendAskShared(sock, palInfo, opttype, attach);
}
/**
 * 向好友发送文件信息.
 * @param sock udp socket
 * @param pal class PalInfo
 * @param opttype 命令额外选项
 * @param extra 扩展数据，即文件信息
 */
void Command::SendFileInfo(int sock, CPPalInfo pal, uint32_t opttype,
                           const char *extra) {
  CreateCommand(opttype | IPMSG_FILEATTACHOPT | IPMSG_SENDMSG, NULL);
  ConvertEncode(pal->encode);
  CreateIpmsgExtra(extra, pal->encode);
  commandSendTo(sock, buf, size, 0, pal->ipv4);
}
void Command::SendFileInfo(int sock, const PalKey& palKey, uint32_t opttype,
                           const char *extra)
{
  auto palInfo = getAndCheckPalInfo(coreThread, palKey);
  return SendFileInfo(sock, palInfo, opttype, extra);
}

/**
 * 发送本人的头像数据.
 * @param sock udp socket
 * @param pal class PalInfo
 */
void Command::SendMyIcon(int sock, CPPalInfo pal, istream& iss) {
  CreateCommand(IPTUX_SENDICON, NULL);
  ConvertEncode(pal->encode);
  CreateIconExtra(iss);
  commandSendTo(sock, buf, size, 0, pal->ipv4);
}

/**
 * 发送本人的签名信息.
 * @param sock udp socket
 * @param pal class PalInfo
 */
void Command::SendMySign(int sock, CPPalInfo pal) {
  auto programData = coreThread.getProgramData();
  CreateCommand(IPTUX_SEND_SIGN, programData->sign.c_str());
  ConvertEncode(pal->encode);
  commandSendTo(sock, buf, size, 0, pal->ipv4);
}

/**
 * 发送底层数据(即发送为最终用户所不能察觉的文件数据).
 * @param sock tcp socket
 * @param pal class PalInfo
 * @param opttype 命令额外选项
 * @param path 文件路径
 */
void Command::SendSublayer(int sock, CPPalInfo pal, uint32_t opttype,
                           const char *path) {
  LOG_DEBUG("send tcp message to %s, op %d, file %s", pal->GetKey().ToString().c_str(), int(opttype), path);
  struct sockaddr_in addr;
  int fd;

  CreateCommand(opttype | IPTUX_SENDSUBLAYER, NULL);
  ConvertEncode(pal->encode);

  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(IPTUX_DEFAULT_PORT);
  addr.sin_addr.s_addr = pal->ipv4;

  if (((connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) &&
       (errno != EINTR)) ||
      (xwrite(sock, buf, size) == -1) || ((fd = open(path, O_RDONLY)) == -1))
  {
    LOG_WARN("send tcp message failed");
    return;
  }

  SendSublayerData(sock, fd);
  close(fd);
}

/**
 * 回馈错误消息.
 * @param pal class PalInfo
 * @param btype 消息归属类型
 * @param error 错误串
 */
void Command::FeedbackError(CPPalInfo pal, GroupBelongType btype,
                            const char *error) {
  MsgPara para;
  ChipData chip;

  /* 构建消息封装包 */
  para.pal = coreThread.GetPal(pal->GetKey());
  para.stype = MessageSourceType::ERROR;
  para.btype = btype;
  chip.type = MESSAGE_CONTENT_TYPE_STRING;
  chip.data = error;
  para.dtlist.push_back(move(chip));
  /* 交给某人处理吧 */
  coreThread.InsertMessage(move(para));
}

/**
 * 将文件描述符数据写入网络套接口.
 * @param sock tcp socket
 * @param fd file descriptor
 */
void Command::SendSublayerData(int sock, int fd) {
  ssize_t len;

  do {
    if ((len = xread(fd, buf, MAX_UDPLEN)) <= 0) break;
    if ((len = xwrite(sock, buf, len)) <= 0) break;
  } while (1);
}

/**
 * 将缓冲区中的字符串转换为指定的编码.
 * @param encode 字符集编码
 */
void Command::ConvertEncode(const string &encode) {
  char *ptr;

  if (encode.empty()) {
    return;
  }

  if (strcasecmp(encode.c_str(), "utf-8") != 0 &&
      (ptr = convert_encode(buf, encode.c_str(), "utf-8"))) {
    size = strlen(ptr) + 1;
    memcpy(buf, ptr, size);
    g_free(ptr);
  }
}

/**
 * 创建命令数据.
 * @param command 命令字
 * @param attach 附加数据
 */
void Command::CreateCommand(uint32_t command, const char *attach) {
  const gchar *env;
  char *ptr;

  snprintf(buf, MAX_UDPLEN, "%s", IPTUX_VERSION);
  size = strlen(buf);
  ptr = buf + size;

  snprintf(ptr, MAX_UDPLEN - size, ":%" PRIu32, packetn);
  packetn++;
  size += strlen(ptr);
  ptr = buf + size;

  env = g_get_user_name();
  snprintf(ptr, MAX_UDPLEN - size, ":%s", env);
  size += strlen(ptr);
  ptr = buf + size;

  env = g_get_host_name();
  snprintf(ptr, MAX_UDPLEN - size, ":%s", env);
  size += strlen(ptr);
  ptr = buf + size;

  if (command == IPMSG_GETFILEDATA) {
    snprintf(ptr, MAX_UDPLEN - size, ":%u", command);
  } else {
    snprintf(ptr, MAX_UDPLEN - size, ":%u", command);
  }
  size += strlen(ptr);
  ptr = buf + size;

  snprintf(ptr, MAX_UDPLEN - size, ":%s", attach ? attach : "");
  size += strlen(ptr) + 1;
}

/**
 * 创建ipmsg的扩展数据(即文件信息).
 * @param extra 扩展数据
 * @param encode 字符集编码
 */
void Command::CreateIpmsgExtra(const char *extra, const char *encode) {
  char *pptr, *ptr;

  pptr = buf + size;
  if (encode && strcasecmp(encode, "utf-8") != 0 &&
      (ptr = convert_encode(extra, encode, "utf-8"))) {
    snprintf(pptr, MAX_UDPLEN - size, "%s", ptr);
    g_free(ptr);
  } else
    snprintf(pptr, MAX_UDPLEN - size, "%s", extra);
  if ((ptr = strrchr(pptr, '\a'))) *(ptr + 1) = '\0';
  size += strlen(pptr) + 1;
}

/**
 * 创建iptux程序独有的扩展数据.
 * @param encode 字符集编码
 */
void Command::CreateIptuxExtra(const string &encode) {
  char *pptr, *ptr;

  auto programData = coreThread.getProgramData();
  pptr = buf + size;
  if (!encode.empty() && strcasecmp(encode.c_str(), "utf-8") != 0 &&
      (ptr = convert_encode(programData->mygroup.c_str(), encode.c_str(),
                            "utf-8"))) {
    snprintf(pptr, MAX_UDPLEN - size, "%s", ptr);
    g_free(ptr);
  } else
    snprintf(pptr, MAX_UDPLEN - size, "%s", programData->mygroup.c_str());
  size += strlen(pptr) + 1;

  pptr = buf + size;
  snprintf(pptr, MAX_UDPLEN - size, "%s", programData->myicon.c_str());
  size += strlen(pptr) + 1;

  pptr = buf + size;
  snprintf(pptr, MAX_UDPLEN - size, "utf-8");
  size += strlen(pptr) + 1;
}

/**
 * 创建个人头像的扩展数据.
 */
void Command::CreateIconExtra(istream& iss) {
  iss.read(buf+size, MAX_UDPLEN-size);
  size+=iss.gcount();
}

}  // namespace iptux
