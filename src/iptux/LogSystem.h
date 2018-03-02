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
#include "iptux/ProgramData.h"

namespace iptux {

class LogSystem {
 public:
  explicit LogSystem(const ProgramData& programData);
  ~LogSystem();

  void CommunicateLog(MsgPara *msgpara, const char *fmt, va_list args);
  void SystemLog(const char *fmt, va_list args);

 private:
  const ProgramData& programData;
  int fdc, fds;

 private:
  void InitSublayer();
};

}  // namespace iptux

#endif
