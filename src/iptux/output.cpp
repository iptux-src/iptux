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

#include <string>

#include "iptux/deplib.h"

using namespace std;

namespace iptux {

string pretty_fname(const string &fname) {
  size_t pos = fname.rfind("/src/");
  if (pos == string::npos) {
    return fname;
  } else {
    return fname.substr(pos + 5);
  }
}

void DoLog(const char *fname, int line, const char *func, GLogLevelFlags level,
           const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  gchar *msg = g_strdup_vprintf(format, ap);
  va_end(ap);
  g_log("iptux", level, "%s:%d:%s:%s", pretty_fname(fname).c_str(), line, func,
        msg);
  g_free(msg);
}

}  // namespace iptux
