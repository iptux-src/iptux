#include "config.h"
#include "GioNotificationService.h"

namespace iptux {

void GioNotificationService::sendNotification(GApplication* app,
                                              const std::string& id,
                                              const std::string& title,
                                              const std::string& body,
                                              const std::string& detailedAction,
                                              GNotificationPriority priority,
                                              GIcon* icon) {
  GNotification* notification = g_notification_new(title.c_str());
  g_notification_set_body(notification, body.c_str());
  g_notification_set_priority(notification, priority);
  if(icon) {
    g_notification_set_icon(notification, icon);
  }
  if(!detailedAction.empty()) {
    g_notification_set_default_action(notification, detailedAction.c_str());
  }
  g_application_send_notification(app, id.c_str(), notification);
  g_object_unref(notification);
}

}  // namespace iptux
