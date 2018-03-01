#include "gtest/gtest.h"

#include "iptux/LogSystem.h"
#include "iptux/TestHelper.h"

using namespace iptux;

TEST(LogSystem, Constructor) {
  auto config = newTestIptuxConfig();
  ProgramData* core = new ProgramData(*config);
  LogSystem* logSystem = new LogSystem(*core);
  delete logSystem;
  delete core;
}
