#include "gtest/gtest.h"

#include "iptux/ProgramDataCore.h"
#include "iptux/TestHelper.h"

using namespace iptux;

TEST(ProgramDataCore, Constructor) {
  auto config = newTestIptuxConfig();
  ProgramDataCore* core = new ProgramDataCore(*config);
  ASSERT_FALSE(core->IsSaveChatHistory());
  core->WriteProgData();
  delete core;
}

