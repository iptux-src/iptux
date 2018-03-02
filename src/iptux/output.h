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

namespace iptux {

#define LOG_DEBUG(format, ...)                                     \
  DoLog(__FILE__, __LINE__, __func__, G_LOG_LEVEL_DEBUG, (format), \
        ##__VA_ARGS__)
#define LOG_INFO(format, ...) \
  DoLog(__FILE__, __LINE__, __func__, G_LOG_LEVEL_INFO, (format), ##__VA_ARGS__)
#define LOG_WARN(format, ...)                                        \
  DoLog(__FILE__, __LINE__, __func__, G_LOG_LEVEL_WARNING, (format), \
        ##__VA_ARGS__)
#define LOG_CRIT(format, ...)                                         \
  DoLog(__FILE__, __LINE__, __func__, G_LOG_LEVEL_CRITICAL, (format), \
        ##__VA_ARGS__)
#define LOG_ERROR(format, ...)                                     \
  DoLog(__FILE__, __LINE__, __func__, G_LOG_LEVEL_ERROR, (format), \
        ##__VA_ARGS__)

/* 警告信息输出 */
#ifndef WARNING
#define pwarning(format, ...) /*warnx(format,##__VA_ARGS__)*/
#else
#define pwarning(format, ...) warnx(format, ##__VA_ARGS__)
#endif

void DoLog(const char *fname, int line, const char *func, GLogLevelFlags level,
           const char *format, ...);
}  // namespace iptux

#endif
