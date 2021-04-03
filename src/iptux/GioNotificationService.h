#ifndef IPTUX_GIO_NOTIFICATION_SERVICE
#define IPTUX_GIO_NOTIFICATION_SERVICE

#include "iptux/NotificationService.h"

namespace iptux {

class GioNotificationService: public NotificationService {
  public:
    ~GioNotificationService() override = default;

    void sendNotification(GApplication* app, std::string id, GNotification* notification) override;
};

}

#endif
