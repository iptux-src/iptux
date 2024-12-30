//
// C++ Implementation: TcpData
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
#include "TcpData.h"

#include <cinttypes>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "iptux-core/internal/CommandMode.h"
#include "iptux-core/internal/SendFile.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

/**
 * 类构造函数.
 */
TcpData::TcpData() : sock(-1), size(0) {}

/**
 * 类析构函数.
 */
TcpData::~TcpData() {
  close(sock);
}

/**
 * TCP连接处理入口.
 * @param sock tcp socket
 */
void TcpData::TcpDataEntry(CoreThread* coreThread, int sock) {
  TcpData tdata;

  tdata.coreThread = coreThread;
  tdata.sock = sock;
  tdata.DispatchTcpData();
}

/**
 * 分派TCP数据处理方案.
 */
void TcpData::DispatchTcpData() {
  struct sockaddr_in addr;
  socklen_t socklen;
  socklen = sizeof(addr);
  getpeername(sock, (struct sockaddr*)&addr, &socklen);
  LOG_DEBUG("received tcp message from %s:%d",
            inAddrToString(addr.sin_addr).c_str(), int(addr.sin_port));

  uint32_t commandno;
  ssize_t len;

  /* 读取消息前缀 */
  if ((len = read_ipmsg_prefix(sock, buf, MAX_SOCKLEN)) <= 0) {
    return;
  }

  /* 分派消息 */
  size = len;  // 设置缓冲区数据的有效长度
  commandno = iptux_get_dec_number(buf, ':', 4);  // 获取命令字
  LOG_INFO("recv TCP request from %s, command NO.: [0x%x] %s",
           inAddrToString(addr.sin_addr).c_str(), commandno,
           CommandMode(GET_MODE(commandno)).toString().c_str());
  switch (GET_MODE(commandno)) {
    case IPMSG_GETFILEDATA:
      RequestData(FileAttr::REGULAR);
      break;
    case IPMSG_GETDIRFILES:
      RequestData(FileAttr::DIRECTORY);
      break;
    case IPTUX_SENDSUBLAYER:
      RecvSublayer(GET_OPT(commandno));
      break;
    default:
      break;
  }
}

/**
 * 请求文件(目录)数据.
 * @param fileattr 文件类型
 */
void TcpData::RequestData(FileAttr fileattr) {
  const char* attachptr;
  char* attach;

  attachptr = iptux_skip_section(buf, ':', 5);
  switch (fileattr) {
    case FileAttr::REGULAR:
      read_ipmsg_filedata(sock, (void*)attachptr, buf + MAX_SOCKLEN - attachptr,
                          buf + size - attachptr);
      break;
    case FileAttr::DIRECTORY:
      read_ipmsg_dirfiles(sock, (void*)attachptr, buf + MAX_SOCKLEN - attachptr,
                          buf + size - attachptr);
      break;
    default:
      break;
  }

  attach = ipmsg_get_attach(buf, ':', 5);
  SendFile::RequestDataEntry(coreThread, sock, fileattr, attach);
  g_free(attach);
}

/**
 * 接收底层数据.
 * @param cmdopt 命令字选项
 */
void TcpData::RecvSublayer(uint32_t cmdopt) {
  static uint32_t count = 0;
  char path[MAX_PATHLEN];
  struct sockaddr_in addr;
  socklen_t len;
  PPalInfo pal;
  int fd;

  /* 检查好友是否存在 */
  len = sizeof(addr);
  getpeername(sock, (struct sockaddr*)&addr, &len);
  if (!(pal = coreThread->GetPal(addr.sin_addr))) {
    return;
  }

  /* 创建即将接收的数据文件路径 */
  switch (GET_OPT(cmdopt)) {
    case IPTUX_PHOTOPICOPT:
      snprintf(path, MAX_PATHLEN, "%s" PHOTO_PATH "/%" PRIx32,
               g_get_user_cache_dir(), inAddrToUint32(pal->ipv4()));
      break;
    case IPTUX_MSGPICOPT:
      snprintf(path, MAX_PATHLEN, "%s" PIC_PATH "/%" PRIx32 "-%" PRIx32 "-%jx",
               g_get_user_cache_dir(), inAddrToUint32(pal->ipv4()), count++,
               (uintmax_t)time(NULL));
      break;
    default:
      snprintf(path, MAX_PATHLEN,
               "%s" IPTUX_PATH "/%" PRIx32 "-%" PRIx32 "-%jx",
               g_get_user_cache_dir(), inAddrToUint32(pal->ipv4()), count++,
               (uintmax_t)time(NULL));
      break;
  }

  LOG_INFO("recv sublayer data from %s, save to %s",
           inAddrToString(pal->ipv4()).c_str(), path);

  /* 终于可以接收数据了^_^ */
  if ((fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
    LOG_ERROR("open file %s failed: %s", path, strerror(errno));
    return;
  }
  RecvSublayerData(fd, strlen(buf) + 1);
  close(fd);

  /* 分派数据 */
  switch (GET_OPT(cmdopt)) {
    case IPTUX_PHOTOPICOPT:
      RecvPhotoPic(pal.get(), path);
      break;
    case IPTUX_MSGPICOPT:
      RecvMsgPic(pal.get(), path);
      break;
    default:
      break;
  }
}

/**
 * 接收数据.
 * @param fd file descriptor
 * @param len 缓冲区无效数据长度
 */
void TcpData::RecvSublayerData(int fd, size_t len) {
  ssize_t ssize;

  if (size != len)
    xwrite(fd, buf + len, size - len);
  do {
    if ((ssize = xread(sock, buf, MAX_SOCKLEN)) <= 0)
      break;
    if ((ssize = xwrite(fd, buf, ssize)) <= 0)
      break;
  } while (1);
}

/**
 * 接收好友形象照片.
 * @param pal class PalInfo
 * @param path file path
 */
void TcpData::RecvPhotoPic(PalInfo* pal, const char* path) {
  g_free(pal->photo);
  pal->photo = g_strdup(path);
  coreThread->Lock();
  coreThread->UpdatePalToList(pal->GetKey());
  coreThread->Unlock();
}

/**
 * 接收消息图片.
 * @param pal class PalInfo
 * @param path file path
 */
void TcpData::RecvMsgPic(PalInfo* pal, const char* path) {
  MsgPara para(coreThread->GetPal(pal->GetKey()));

  /* 构建消息封装包 */
  para.stype = IptuxMsgSrcType::IPTUX_MSG_SRC_PAL;
  para.btype = GROUP_BELONG_TYPE_REGULAR;
  ChipData chip(MESSAGE_CONTENT_TYPE_PICTURE, path);
  para.dtlist.push_back(chip);

  /* 交给某人处理吧 */
  coreThread->InsertMessage(std::move(para));
}

}  // namespace iptux
