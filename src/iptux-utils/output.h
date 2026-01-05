//
// C++ Interface: output
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_OUTPUT_H
#define IPTUX_OUTPUT_H

#include <glib.h>

#define LOG_DEBUG(...) \
  iptux::DoLog(__FILE__, __LINE__, __func__, G_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_INFO(...) \
  iptux::DoLog(__FILE__, __LINE__, __func__, G_LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_WARN(...) \
  iptux::DoLog(__FILE__, __LINE__, __func__, G_LOG_LEVEL_WARNING, __VA_ARGS__)
#define LOG_CRIT(...) \
  iptux::DoLog(__FILE__, __LINE__, __func__, G_LOG_LEVEL_CRITICAL, __VA_ARGS__)
#define LOG_ERROR(...) \
  iptux::DoLog(__FILE__, __LINE__, __func__, G_LOG_LEVEL_ERROR, __VA_ARGS__)
namespace iptux {

/* 警告信息输出 */
#ifndef WARNING
#define pwarning(format, ...) /*warnx(format,##__VA_ARGS__)*/
#else
#define pwarning(format, ...) warnx(format, ##__VA_ARGS__)
#endif

enum class LogLevel {
  WARN = G_LOG_LEVEL_WARNING,
  INFO = G_LOG_LEVEL_INFO,
  DEBUG = G_LOG_LEVEL_DEBUG
};

class Log {
 public:
  /**
   * @brief Set the Log Level object
   *
   * @param level
   */
  static void setLogLevel(LogLevel level);
  static LogLevel getLogLevel();
  static bool IsDebugEnabled();
  static bool IsInfoEnabled();
  static bool IsWarnEnabled();
};

void DoLog(const char* fname,
           int line,
           const char* func,
           GLogLevelFlags level,
           const char* format,
           ...) G_GNUC_PRINTF(5, 6);
}  // namespace iptux

#endif
