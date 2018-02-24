#include "gtest/gtest.h"

#include "iptux/ProgramData.h"
#include "iptux/TestHelper.h"

using namespace iptux;

TEST(ProgramData, Constructor) {
  auto config = newTestIptuxConfig();
  ProgramData* core = new ProgramData(*config);
  ASSERT_FALSE(core->IsSaveChatHistory());
  core->WriteProgData();
  delete core;
}

