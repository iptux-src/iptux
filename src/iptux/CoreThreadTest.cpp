#include "gtest/gtest.h"

#include "iptux/CoreThread.h"
#include "iptux/TestHelper.h"

using namespace iptux;

TEST(CoreThread, Constructor) {
  auto config = newTestIptuxConfig();
  ProgramDataCore* core = new ProgramDataCore(*config);
  core->sign = "abc";
  CoreThread* thread = new CoreThread(*core);
  thread->start();
  thread->stop();
  delete thread;
  delete core;
}


