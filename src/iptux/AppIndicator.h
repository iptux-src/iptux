// license: GPL v2
#pragma once

#include "iptux/Application.h"

namespace iptux {
class IptuxAppIndicatorPrivate;
class IptuxAppIndicator {
 public:
  IptuxAppIndicator(Application* app);

 private:
  std::shared_ptr<IptuxAppIndicatorPrivate> priv;
};
}  // namespace iptux
