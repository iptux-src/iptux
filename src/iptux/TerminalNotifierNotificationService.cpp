#include "config.h"
#include "iptux/TerminalNotifierNotificationService.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"

#include <vector>

using namespace std;

namespace iptux {

void TerminalNotifierNoticationService::sendNotification(
    GApplication* app,
    const std::string& id,
    const std::string& title,
    const std::string& body,
    const std::string&,
    GNotificationPriority,
    GIcon*) {
  vector<string> args = {"terminal-notifier",
                         "-group",
                         id,
                         "-title",
                         title,
                         "-subtitle",
                         g_application_get_application_id(app),
                         "-message",
                         body};
  char** args2 = new char*[args.size() + 1];
  for (int i = 0; i < int(args.size()); ++i) {
    args2[i] = g_strdup(args[i].c_str());
  }
  args2[args.size()] = nullptr;
  GError* error = nullptr;
  gint exit_status = 0;
  gboolean res = g_spawn_sync(
      nullptr, args2, nullptr,
      (GSpawnFlags)(G_SPAWN_SEARCH_PATH | G_SPAWN_FILE_AND_ARGV_ZERO |
                    G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL),
      nullptr, nullptr, nullptr, nullptr, &exit_status, &error);
  if (!res) {
    LOG_WARN("g_spawn_sync failed: %s-%d-%s", g_quark_to_string(error->domain),
             error->code, error->message);
  }
  if (!g_spawn_check_exit_status(exit_status, &error)) {
    LOG_WARN("g_spawn_sync failed: %s-%d-%s", g_quark_to_string(error->domain),
             error->code, error->message);
  }
}

}  // namespace iptux
