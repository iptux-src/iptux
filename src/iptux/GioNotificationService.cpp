#include "config.h"
#include "GioNotificationService.h"

namespace iptux {

void GioNotificationService::sendNotification(GApplication* app, std::string id, GNotification* notification) {
  g_application_send_notification(app, id.c_str(), notification);
}

}
