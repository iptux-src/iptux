#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux/LogSystem.h"

using namespace std;
using namespace iptux;

TEST(LogSystem, Constructor) {
  auto config = newTestIptuxConfig();
  auto core = make_shared<ProgramData>(config);
  auto logSystem = make_shared<LogSystem>(core);
}
