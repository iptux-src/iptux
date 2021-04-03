#ifndef IPTUX_NOTIFICATION_SERVICE
#define IPTUX_NOTIFICATION_SERVICE

#include <string>

#include <gio/gio.h>

namespace iptux {

class NotificationService {
  public:
    virtual ~NotificationService() = default;

    virtual void sendNotification(GApplication* app, std::string id, GNotification* notification) = 0;
};

}

#endif
