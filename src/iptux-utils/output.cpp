//
// C++ Implementation: output
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
#include "output.h"
#include "iptux-utils/utils.h"
#include <pthread.h>
#include <sstream>
#include <string>

#include <sys/time.h>

#include <unistd.h>

using namespace std;

namespace iptux {

static LogLevel _level = LogLevel::WARN;

static string getThreadName() {
  ostringstream oss;
  oss << pthread_self();
  return oss.str();
}

static char logLevelAsChar(GLogLevelFlags logLevel) {
  switch (logLevel) {
    case G_LOG_LEVEL_DEBUG:
      return 'D';
    case G_LOG_LEVEL_INFO:
      return 'I';
    case G_LOG_LEVEL_MESSAGE:
      return 'M';
    case G_LOG_LEVEL_WARNING:
      return 'W';
    case G_LOG_LEVEL_ERROR:
      return 'E';
    default:
      return 'U';
  }
}

static string nowAsString() {
  GDateTime* dt = g_date_time_new_now_local();
  char* p =  g_date_time_format(dt, "%H:%M:%S.%f");
  if(!p) {
    g_date_time_unref(dt);
    return "00:00:00.000000";
  }
  string res = p;
  g_free(p);
  g_date_time_unref(dt);
  return res;
}

string pretty_fname(const string& fname) {
  size_t pos = fname.rfind("/");
  if (pos == string::npos) {
    return fname;
  } else {
    return fname.substr(pos + 1);
  }
}

void DoLog(const char* fname,
           int line,
           const char* func,
           GLogLevelFlags level,
           const char* format,
           ...) {
  if (int(level) > int(_level)) {
    return;
  }
  va_list ap;
  va_start(ap, format);
  gchar* msg = g_strdup_vprintf(format, ap);
  va_end(ap);
  fprintf(stderr, "[%s][%s][%c]%s:%d:%s:%s\n", nowAsString().c_str(),
          getThreadName().c_str(), logLevelAsChar(level),
          pretty_fname(fname).c_str(), line, func, msg);
  g_free(msg);
}

bool Log::IsDebugEnabled() {
  return LogLevel::DEBUG <= _level;
}

bool Log::IsInfoEnabled() {
  return LogLevel::INFO <= _level;
}

void Log::setLogLevel(LogLevel level) {
  _level = level;
}

LogLevel Log::getLogLevel() {
  return _level;
}

}  // namespace iptux
