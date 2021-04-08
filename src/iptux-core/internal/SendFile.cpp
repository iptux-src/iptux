//
// C++ Implementation: SendFile
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
#include "SendFile.h"

#include <cinttypes>
#include <cstring>
#include <memory>

#include <sys/socket.h>
#include <unistd.h>

#include "iptux-core/internal/AnalogFS.h"
#include "iptux-core/internal/Command.h"
#include "iptux-core/internal/SendFileData.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

SendFile::SendFile(CoreThread* coreThread) : coreThread(coreThread) {}

SendFile::~SendFile() {}

/**
 * 发送本机共享文件信息入口.
 * @param pal class PalInfo
 */
void SendFile::SendSharedInfoEntry(CoreThread* coreThread, PPalInfo pal) {
  coreThread->Lock();
  auto fileInfos = coreThread->getProgramData()->GetSharedFileInfos();
  SendFile(coreThread).SendFileInfo(pal, IPTUX_SHAREDOPT, fileInfos);
  coreThread->Unlock();
}

/**
 * 广播文件信息入口.
 * @param plist 好友链表
 * @param flist 文件信息链表
 * @note 文件路径链表中的数据将被本函数处理掉
 */
void SendFile::BcstFileInfoEntry(CoreThread* coreThread,
                                 const vector<const PalInfo*>& pals,
                                 const std::vector<FileInfo*>& files) {
  SendFile(coreThread).BcstFileInfo(pals, 0, files);
}

/**
 * 请求文件数据入口.
 * @param sock tcp socket
 * @param fileattr 文件类型
 * @param attach 附加数据
 */
void SendFile::RequestDataEntry(CoreThread* coreThread,
                                int sock,
                                FileAttr fileattr,
                                char* attach) {
  struct sockaddr_in addr;
  socklen_t len;
  uint32_t fileid;
  uint32_t filectime;
  /* 检查文件属性是否匹配 */

  fileid = iptux_get_hex_number(attach, ':', 1);
  auto file = coreThread->GetPrivateFileById(fileid);
  /* 兼容windows版信鸽(IPMSG) ,这里的信鸽不是飞鸽传书(IPMSG)*/
  if (!file) {
    fileid = iptux_get_dec_number(attach, ':', 1);
    file = coreThread->GetPrivateFileById(fileid);
  }
  /* 兼容android版信鸽(IPMSG) */
  if (!file) {
    fileid = iptux_get_hex_number(attach, ':', 0);
    filectime = iptux_get_dec_number(attach, ':', 1);
    file = coreThread->GetPrivateFileByPacketN(fileid, filectime);
  }
  if (!file || file->fileattr != fileattr)
    return;
  /* 检查好友数据是否存在 */
  len = sizeof(addr);
  getpeername(sock, (struct sockaddr*)&addr, &len);
  if (!(coreThread->GetPal(addr.sin_addr))) {
    LOG_INFO("Pal not exist: %s", inAddrToString(addr.sin_addr).c_str());
    return;
  }

  /* 发送文件数据 */
  //        /**
  //         *文件信息可能被删除或修改，必须单独复制一份.
  //         */
  //        file->fileown = pal;
  SendFile(coreThread).ThreadSendFile(sock, file);
}

/**
 * 发送文件信息.
 * @param pal class PalInfo
 * @param opttype 命令字选项
 * @param filist 文件信息链表
 */
void SendFile::SendFileInfo(PPalInfo pal,
                            uint32_t opttype,
                            vector<FileInfo>& fileInfos) {
  AnalogFS afs;
  Command cmd(*coreThread);
  char buf[MAX_UDPLEN];
  size_t len;
  char *ptr, *name;

  /* 初始化 */
  len = 0;
  ptr = buf;
  buf[0] = '\0';

  /* 将文件信息写入缓冲区 */
  for (FileInfo& fileInfo : fileInfos) {
    auto file = &fileInfo;
    if (access(file->filepath, F_OK) == -1) {
      continue;
    }
    name = ipmsg_get_filename_pal(file->filepath);  //获取面向好友的文件名
    file->filesize = afs.ftwsize(file->filepath);  //不得不计算文件长度了
    file->packetn = cmd.Packetn();
    snprintf(ptr, MAX_UDPLEN - len,
             "%" PRIu32 ":%s:%" PRIx64 ":%" PRIx32 ":%" PRIx32 ":\a",
             file->fileid, name, file->filesize, file->filectime,
             file->fileattr);
    g_free(name);
    len += strlen(ptr);
    ptr = buf + len;
  }

  /* 发送文件信息 */
  cmd.SendFileInfo(coreThread->getUdpSock(), pal->GetKey(), opttype, buf);
}

/**
 * 广播文件信息.
 * @param plist 好友链表
 * @param opttype 命令字选项
 * @param filist 文件信息链表
 */
void SendFile::BcstFileInfo(const std::vector<const PalInfo*>& pals,
                            uint32_t opttype,
                            const std::vector<FileInfo*>& files) {
  AnalogFS afs;
  Command cmd(*coreThread);
  char buf[MAX_UDPLEN];
  size_t len;
  char* ptr;

  for (auto pal : pals) {
    /* 初始化 */
    len = 0;
    ptr = buf;
    buf[0] = '\0';
    vector<string> buffer;
    for (auto file : files) {
      if (!(file->fileown->GetKey() == pal->GetKey()))
        continue;
      if (access(file->filepath, F_OK) == -1)
        continue;
      file->filesize = afs.ftwsize(file->filepath);  //不得不计算文件长度了
      file->packetn = cmd.Packetn();

      buffer.push_back(Command::encodeFileInfo(*file));
    }

    string res;
    for (auto b : buffer) {
      if (res.length() + b.length() > MAX_UDPLEN)
        break;
      res += b;
    }

    cmd.SendFileInfo(coreThread->getUdpSock(), pal->GetKey(), opttype,
                     res.c_str());
  }
}

/**
 * 发送文件数据.
 * @param sock tcp socket
 * @param file 文件信息
 */
void SendFile::ThreadSendFile(int sock, PFileInfo file) {
  auto sfdt = make_shared<SendFileData>(coreThread, sock, file);
  coreThread->RegisterTransTask(sfdt);
  sfdt->SendFileDataEntry();
}

}  // namespace iptux
