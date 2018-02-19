//
// C++ Interface: LogSystem
//
// Description:
// 相关日志记录
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_LOGSYSTEM_H
#define IPTUX_LOGSYSTEM_H

#include "iptux/Models.h"

namespace iptux {

class LogSystem {
 public:
  LogSystem();
  ~LogSystem();

  void InitSublayer();
  void CommunicateLog(MsgPara *msgpara, const char *fmt, ...);
  void SystemLog(const char *fmt, ...);

 private:
  int fdc, fds;
};

}  // namespace iptux

#endif
