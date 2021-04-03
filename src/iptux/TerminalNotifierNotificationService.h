#ifndef IPTUX_TERMINAL_NOTIFIER_NOTIFICATION_SERVICE
#define IPTUX_TERMINAL_NOTIFIER_NOTIFICATION_SERVICE

#include "iptux/NotificationService.h"

namespace iptux {

class TerminalNotifierNoticationService : public NotificationService {
 public:
  ~TerminalNotifierNoticationService() override = default;

  void sendNotification(GApplication* app,
                        const std::string& id,
                        const std::string& title,
                        const std::string& body,
                        GNotificationPriority priority,
                        GIcon* icon) override;
};

}  // namespace iptux

#endif
