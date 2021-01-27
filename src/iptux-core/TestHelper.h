#ifndef IPTUX_CORE_TESTHELPER_H
#define IPTUX_CORE_TESTHELPER_H

#include <memory>
#include <string>

#include "iptux-utils/TestHelper.h"
#include "iptux-core/IptuxConfig.h"
#include "iptux-core/CoreThread.h"

namespace iptux {
  std::shared_ptr<IptuxConfig> newTestIptuxConfig();
  std::shared_ptr<IptuxConfig> newTestIptuxConfigWithFile();
  std::string testDataPath(const std::string& fname);

  std::shared_ptr<CoreThread> newCoreThreadOnIp(const std::string& ip);

  using PIptuxConfig = std::shared_ptr<IptuxConfig>;
  using PCoreThread = std::shared_ptr<CoreThread>;

  std::tuple<PCoreThread, PCoreThread>
  initAndConnnectThreadsFromConfig(PIptuxConfig c1, PIptuxConfig c2);
}


#endif //IPTUX_TESTHELPER_H
