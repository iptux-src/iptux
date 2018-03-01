#include "gtest/gtest.h"

#include "iptux/UiCoreThread.h"
#include "iptux/TestHelper.h"

using namespace iptux;

TEST(UiCoreThread, Constructor) {
  auto config = newTestIptuxConfig();
  ProgramData* core = new ProgramData(*config);
  core->sign = "abc";
  UiCoreThread* thread = new UiCoreThread(*core);
  thread->start();
  thread->stop();
  delete thread;
  delete core;
}
