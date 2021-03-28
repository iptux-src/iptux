//
// C++ Interface: RecvFile
//
// Description:
// 接受相关的文件信息,不包含文件数据
//
// Author: cwll <cwll2009@126.com> ,(C) 2012.02
//        Jally <jallyx@163.com>, (C) 2008
// 2012.02:把文件接收确认和选择放在了聊天窗口，所以这个类的大部分功能都不用了
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_RECVFILE_H
#define IPTUX_RECVFILE_H

#include <string>

#include "iptux-core/CoreThread.h"
#include "iptux-core/Models.h"

namespace iptux {

class RecvFile {
 private:
  RecvFile();
  ~RecvFile();

 public:
  static void RecvEntry(CoreThread* coreThread,
                        PPalInfo pal,
                        const std::string extra,
                        int packeno);

 private:
  void ParseFilePara(GData** para);
  FileInfo* DivideFileinfo(char** extra);
};

}  // namespace iptux

#endif
