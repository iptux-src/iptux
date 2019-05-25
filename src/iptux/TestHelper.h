#ifndef IPTUX_TESTHELPER_H
#define IPTUX_TESTHELPER_H

#include <memory>
#include <string>

#include "iptux/IptuxConfig.h"
#include "iptux/CoreThread.h"

namespace iptux {
  std::shared_ptr<IptuxConfig> newTestIptuxConfig();
  std::shared_ptr<IptuxConfig> newTestIptuxConfigWithFile();
  std::string readTestData(const std::string& fname);
  std::string testDataPath(const std::string& fname);

  std::shared_ptr<CoreThread> newCoreThreadOnIp(const std::string& ip);
}


#endif //IPTUX_TESTHELPER_H
