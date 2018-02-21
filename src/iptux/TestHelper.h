#ifndef IPTUX_TESTHELPER_H
#define IPTUX_TESTHELPER_H

#include <memory>

#include "iptux/IptuxConfig.h"

namespace iptux {
  std::shared_ptr<IptuxConfig> newTestIptuxConfig();
}


#endif //IPTUX_TESTHELPER_H
