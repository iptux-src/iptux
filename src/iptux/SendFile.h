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

#include "iptux/mess.h"

namespace iptux {

class SendFile {
 public:
  SendFile();
  ~SendFile();

  void SendSharedInfoEntry(PalInfo *pal);
  void SendFileInfoEntry(PalInfo *pal, GSList *flist);
  void BcstFileInfoEntry(GSList *plist, GSList *flist);
  void RequestDataEntry(int sock, uint32_t fileattr, char *attach);

 private:
  void SendFileInfo(PalInfo *pal, uint32_t opttype, GSList *filist);
  void BcstFileInfo(GSList *plist, uint32_t opttype, GSList *filist);
  void ThreadSendFile(int sock, FileInfo *file);
};

}  // namespace iptux

#endif
