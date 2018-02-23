#include "gtest/gtest.h"

#include "iptux/UiCoreThread.h"
#include "iptux/TestHelper.h"

using namespace iptux;

TEST(CoreThread, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  ProgramDataCore* core = new ProgramDataCore(*config);
  UiCoreThread* thread = new UiCoreThread(*core);
  thread->setDebug(true);
  ASSERT_EQ(thread->GetMsglineItems(), 0);
  thread->start();
  thread->stop();
  delete thread;
  delete core;
}


