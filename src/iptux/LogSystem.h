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

#include <memory>

#include "iptux-core/Models.h"
#include "iptux-core/ProgramData.h"

namespace iptux {

class LogSystem {
 public:
  explicit LogSystem(std::shared_ptr<const ProgramData> programData);
  ~LogSystem();

  void communicateLog(const MsgPara* msgpara, const char* fmt, ...)
      G_GNUC_PRINTF(3, 4);
  void systemLog(const char* fmt, ...) G_GNUC_PRINTF(2, 3);
  void communicateLogv(const MsgPara* msgpara, const char* fmt, va_list args);
  void systemLogv(const char* fmt, va_list args);

  std::string getChatLogPath() const;
  std::string getSystemLogPath() const;

 private:
  std::shared_ptr<const ProgramData> programData;
  int fdc, fds;

 private:
  void InitSublayer();
};

}  // namespace iptux

#endif
