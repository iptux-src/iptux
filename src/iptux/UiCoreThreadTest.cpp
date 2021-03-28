#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux/UiCoreThread.h"

using namespace std;
using namespace iptux;

TEST(UiCoreThread, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  auto core = make_shared<UiProgramData>(config);
  core->sign = "abc";

  Application app(config);
  app.startup();
  app.activate();

  UiCoreThread* thread = new UiCoreThread(&app, core);
  thread->SystemLog("hello %s", "world");
  thread->start();
  thread->stop();
  delete thread;
}
