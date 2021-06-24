#ifndef IPTUX_NOTIFICATION_SERVICE
#define IPTUX_NOTIFICATION_SERVICE

#include <string>

#include <gio/gio.h>

namespace iptux {

class NotificationService {
 public:
  virtual ~NotificationService() = default;

  virtual void sendNotification(GApplication* app,
                                const std::string& id,
                                const std::string& title,
                                const std::string& body,
                                const std::string& detailedAction,
                                GNotificationPriority priority,
                                GIcon* icon) = 0;
};

}  // namespace iptux

#endif
