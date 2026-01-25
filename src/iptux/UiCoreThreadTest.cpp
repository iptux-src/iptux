#include "Application.h"
#include "gtest/gtest.h"

#include "iptux/TestHelper.h"
#include "iptux/UiCoreThread.h"

using namespace std;
using namespace iptux;

TEST(UiCoreThread, Constructor) {
  Application* app = CreateApplication();
  if (!app) {
    GTEST_SKIP() << "Application creation failed";
  }
  auto core = make_shared<ProgramData>(app->getConfig());
  core->sign = "abc";
  UiCoreThread* thread = new UiCoreThread(app, core);
  thread->setIgnoreTcpBindFailed(true);
  thread->start();
  thread->stop();
  delete thread;
  DestroyApplication(app);
}
