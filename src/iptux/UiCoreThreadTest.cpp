#include "gtest/gtest.h"

#include "iptux/UiCoreThread.h"
#include "iptux/TestHelper.h"

using namespace iptux;

TEST(UiCoreThread, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  UiProgramData* core = new UiProgramData(*config);
  core->sign = "abc";
  UiCoreThread* thread = new UiCoreThread(*core);
  thread->SystemLog("hello %s", "world");
  thread->start();
  thread->stop();
  delete thread;
  delete core;
}
