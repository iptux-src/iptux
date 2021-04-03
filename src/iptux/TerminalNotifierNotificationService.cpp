#include "config.h"
#include "iptux/TerminalNotifierNotificationService.h"
#include "iptux-utils/utils.h"

#include <iostream>

using namespace std;

namespace iptux {

void TerminalNotifierNoticationService::sendNotification(
    GApplication* app,
    const std::string& id,
    const std::string& title,
    const std::string& body,
    GNotificationPriority priority,
    GIcon* icon) {
  cout << stringFormat(
              "terminal-notifier -group \"%s\" -title \"%s\" -subtitle \"%s\" "
              "-message \"%s\"",
              id.c_str(), title.c_str(), g_application_get_application_id(app),
              body.c_str())
       << endl;
}

}  // namespace iptux
