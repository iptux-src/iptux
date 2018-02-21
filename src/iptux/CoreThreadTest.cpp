#include "gtest/gtest.h"

#include "iptux/CoreThread.h"
#include "iptux/TestHelper.h"

using namespace iptux;

TEST(CoreThread, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  ProgramDataCore* core = new ProgramDataCore(*config);
  CoreThread* thread = new CoreThread(*core);
  ASSERT_EQ(thread->GetMsglineItems(), 0);
  delete thread;
  delete core;
}


