#pragma once

#include "iptux/Application.h"

namespace iptux {
class IptuxAppIndicatorPrivate;
class IptuxAppIndicator {
 public:
  IptuxAppIndicator(Application* app);
  void SetUnreadCount(int count);

 private:
  std::shared_ptr<IptuxAppIndicatorPrivate> priv;
};
}  // namespace iptux
