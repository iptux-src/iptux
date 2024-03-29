#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux/UiCoreThread.h"

using namespace std;
using namespace iptux;

TEST(UiCoreThread, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  auto core = make_shared<ProgramData>(config);
  core->sign = "abc";

  Application app(config);
  app.startup();
  app.activate();

  UiCoreThread* thread = new UiCoreThread(&app, core);
  thread->start();
  thread->stop();
  delete thread;
}
