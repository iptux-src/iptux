#include "Iptux.h"

#include "config.h"
#include <string>

#include <glib.h>
#include <libintl.h>

#include <glib/gi18n.h>
#include <glog/logging.h>

#include "iptux-core/IptuxConfig.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"

using namespace std;
using namespace iptux;

static gboolean version = FALSE;
static gchar* configFilename = nullptr;
static gchar* logger = nullptr;
static gchar* bindIp = nullptr;
static GLogLevelFlags logLevel = G_LOG_LEVEL_WARNING;

string getConfigPath() {
  const char* res1;
  if (bindIp == nullptr) {
    res1 = g_build_path("/", g_getenv("HOME"), ".iptux", "config.json", NULL);
  } else {
    res1 = g_build_path("/", g_getenv("HOME"), ".iptux",
                        stringFormat("config.%s.json", bindIp).c_str(), NULL);
  }
  string res2(res1);
  g_free(gpointer(res1));
  return res2;
}

static GOptionEntry entries[] = {
    {"version", 'v', 0, G_OPTION_ARG_NONE, &version,
     "Output version information and exit", NULL},
    {"config", 'c', 0, G_OPTION_ARG_FILENAME, &configFilename,
     "Specify config path", "CONFIG_PATH"},
    {"log", 'l', 0, G_OPTION_ARG_STRING, &logger,
     "Specify log level: DEBUG, INFO, WARN, default is WARN", "LEVEL"},
    {"bind", 'b', 0, G_OPTION_ARG_STRING, &bindIp,
     "Specify bind IP, like 127.0.0.2", "IP"},
    {nullptr, 0, 0, G_OPTION_ARG_NONE, nullptr, nullptr, nullptr}};

static string nowAsString() {
  time_t rawtime;
  struct tm timeinfo;
  char buffer[80];

  time(&rawtime);
  localtime_r(&rawtime, &timeinfo);

  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
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

static void logHandler(const gchar* log_domain,
                       GLogLevelFlags log_level,
                       const gchar* message,
                       gpointer)

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

  if (logger != nullptr) {
    logStr = logger;
  }

  g_log_set_handler("iptux",
                    GLogLevelFlags(G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL |
                                   G_LOG_FLAG_RECURSION),
                    logHandler, NULL);

  if (logStr == "DEBUG") {
    Log::setLogLevel(LogLevel::DEBUG);
    logLevel = G_LOG_LEVEL_DEBUG;
  } else if (logStr == "INFO") {
    Log::setLogLevel(LogLevel::INFO);
    logLevel = G_LOG_LEVEL_INFO;
  } else if (logStr == "WARN") {
    Log::setLogLevel(LogLevel::WARN);
    logLevel = G_LOG_LEVEL_WARNING;
  } else {
    LOG_ERROR("unknown log level: %s", logger);
  }
}

int main(int argc, char** argv) {
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
  auto config = make_shared<IptuxConfig>(configPath);
  dealLog(*config);
  if (bindIp) {
    config->SetString("bind_ip", bindIp);
  }
  auto app = Iptux::create(config);
  return app->run(argc, argv);
}
