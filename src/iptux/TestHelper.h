#ifndef IPTUX_TESTHELPER_H
#define IPTUX_TESTHELPER_H

#include <memory>
#include <string>

#include "iptux/IptuxConfig.h"

namespace iptux {
  std::shared_ptr<IptuxConfig> newTestIptuxConfig();
  std::string readTestData(const std::string& fname);
}


#endif //IPTUX_TESTHELPER_H
