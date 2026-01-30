#pragma once

#include <gio/gio.h>
#include <sigc++/sigc++.h>

#include <memory>

#include "iptux-core/StatusIconMode.h"

namespace iptux {
class IptuxAppIndicatorPrivate;
class IptuxAppIndicator {
 public:
  IptuxAppIndicator(GActionGroup* action_group);
  void SetUnreadCount(int count);
  void SetMode(StatusIconMode mode);
  void StopBlinking();

  sigc::signal<void> sigActivateMainWindow;

 private:
  std::shared_ptr<IptuxAppIndicatorPrivate> priv;
};
}  // namespace iptux
