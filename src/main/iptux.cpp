/***************************************************************************
 *   Copyright (C) 2008 by Jally   *
 *   jallyx@163.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "iptux/config.h"
#include <string>

#include <glib.h>
#include <libintl.h>
#include <execinfo.h>

#include "iptux/Application.h"
#include "iptux/SoundSystem.h"
#include "iptux/deplib.h"
#include "iptux/output.h"

using namespace std;
using namespace iptux;

string getConfigPath() {
  const char* res1 =
      g_build_path("/", g_getenv("HOME"), ".iptux", "config.json", NULL);
  string res2(res1);
  g_free(gpointer(res1));
  return res2;
}

static gboolean version = FALSE;
static gchar* configFilename = nullptr;
static gchar* log = nullptr;
static GLogLevelFlags logLevel = G_LOG_LEVEL_WARNING;

static GOptionEntry entries[] = {
    {"version", 'v', 0, G_OPTION_ARG_NONE, &version,
     "Output version information and exit", NULL},
    {"config", 'c', 0, G_OPTION_ARG_FILENAME, &configFilename,
     "Specify config path", "CONFIG_PATH"},
    {"log", 'l', 0, G_OPTION_ARG_STRING, &log,
     "Specify log level: DEBUG, INFO, WARN, default is WARN", "LEVEL"},
    {NULL}};

static string nowAsString() {
  time_t rawtime;
  struct tm* timeinfo;
  char buffer[80];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  return buffer;
}

static const char* logLevelAsString(GLogLevelFlags logLevel) {
  switch (logLevel) {
    case G_LOG_LEVEL_DEBUG:
      return "DEBUG";
    case G_LOG_LEVEL_INFO:
      return "INFO ";
    case G_LOG_LEVEL_MESSAGE:
      return "MESSA";
    case G_LOG_LEVEL_WARNING:
      return "WARN ";
    case G_LOG_LEVEL_ERROR:
      return "ERROR";
    default:
      return "UNKNO";
  }
}

static void logHandler(const gchar* log_domain, GLogLevelFlags log_level,
                       const gchar* message, gpointer user_data)

{
  if (log_level > logLevel) {
    return;
  }

  fprintf(stderr, "[%s][%s][%s]%s\n", nowAsString().c_str(), log_domain,
          logLevelAsString(log_level), message);
}

static void dealLog(const IptuxConfig& config) {
  string logStr = "WARN";

  if (!config.GetString("log_level").empty()) {
    logStr = config.GetString("log_level");
  }

  if (log != nullptr) {
    logStr = log;
  }

  g_log_set_handler("iptux",
                    GLogLevelFlags(G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL |
                                   G_LOG_FLAG_RECURSION),
                    logHandler, NULL);

  if (logStr == "DEBUG") {
    logLevel = G_LOG_LEVEL_DEBUG;
  } else if (logStr == "INFO") {
    logLevel = G_LOG_LEVEL_INFO;
  } else if (logStr == "WARN") {
    logLevel = G_LOG_LEVEL_WARNING;
  } else {
    LOG_ERROR("unknown log level: %s", log);
  }
}



static void segvHandler(int sig) {
  void *array[99];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 99);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}


int main(int argc, char** argv) {
  signal(SIGSEGV, segvHandler);
  signal(SIGABRT, segvHandler);
  signal(SIGTRAP, segvHandler);
  setlocale(LC_ALL, "");
  bindtextdomain(GETTEXT_PACKAGE, __LOCALE_PATH);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

  GError* error = NULL;
  GOptionContext* context;

  context = g_option_context_new(_("- A software for sharing in LAN"));
  g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_print(_("option parsing failed: %s\n"), error->message);
    exit(1);
  }
  if (version) {
    printf("iptux: " VERSION "\n");
    exit(0);
  }
  string configPath = configFilename ? configFilename : getConfigPath();
  IptuxConfig config(configPath);
  dealLog(config);

  Application app(config);
  return app.run(argc, argv);
}
