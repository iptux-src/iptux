#include "gtest/gtest.h"

#include "iptux/LogSystem.h"
#include "iptux-core/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(LogSystem, Constructor) {
  auto config = newTestIptuxConfig();
  auto core = make_shared<ProgramData>(config);
  LogSystem* logSystem = new LogSystem(core);
  delete logSystem;
}
