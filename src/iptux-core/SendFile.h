//
// C++ Interface: SendFile
//
// Description:
// 发送相关的文件信息,不包含文件数据
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_SENDFILE_H
#define IPTUX_SENDFILE_H

#include <vector>

#include "iptux-core/Models.h"
#include "iptux-core/CoreThread.h"

namespace iptux {

class SendFile {
 private:
  explicit SendFile(CoreThread* coreThread);
  ~SendFile();

 public:
  static void SendSharedInfoEntry(CoreThread* coreThread, PPalInfo pal);
  static void BcstFileInfoEntry(CoreThread* coreThread, GSList *plist, GSList *flist);
  static void RequestDataEntry(CoreThread* coreThread, int sock, FileAttr fileattr, char *attach);

 private:
  void SendFileInfo(PPalInfo pal, uint32_t opttype, std::vector<FileInfo>& filist);
  void BcstFileInfo(GSList *plist, uint32_t opttype, GSList *filist);
  void ThreadSendFile(int sock, PFileInfo file);

 private:
  CoreThread* coreThread;
};

}  // namespace iptux

#endif
