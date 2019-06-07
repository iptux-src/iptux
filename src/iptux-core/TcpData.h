//
// C++ Interface: TcpData
//
// Description:
// 对TCP连接进行处理
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_TCPDATA_H
#define IPTUX_TCPDATA_H

#include "iptux-core/ipmsg.h"
#include "iptux-core/Models.h"
#include "iptux-core/CoreThread.h"

namespace iptux {

class TcpData {
 public:
  TcpData();
  ~TcpData();

  static void TcpDataEntry(CoreThread* coreThread, int sock);

 private:
  void DispatchTcpData();

  void RequestData(uint32_t fileattr);
  void RecvSublayer(uint32_t cmdopt);

  void RecvSublayerData(int fd, size_t len);
  void RecvPhotoPic(PalInfo *pal, const char *path);
  void RecvMsgPic(PalInfo *pal, const char *path);

  CoreThread* coreThread;
  int sock;               //数据交流套接口
  size_t size;            //缓冲区已使用长度
  char buf[MAX_SOCKLEN];  //缓冲区
};

}  // namespace iptux

#endif
