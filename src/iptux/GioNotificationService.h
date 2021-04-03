#ifndef IPTUX_GIO_NOTIFICATION_SERVICE
#define IPTUX_GIO_NOTIFICATION_SERVICE

#include "iptux/NotificationService.h"

namespace iptux {

class GioNotificationService : public NotificationService {
 public:
  ~GioNotificationService() override = default;

  void sendNotification(GApplication* app,
                        const std::string& id,
                        const std::string& title,
                        const std::string& body,
                        GNotificationPriority priority,
                        GIcon* icon) override;
};

}  // namespace iptux

#endif
