#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux/MainWindow.h"

using namespace std;
using namespace iptux;

TEST(MainWindow, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  Application app(config);
  app.startup();

  MainWindow mw(&app, *app.getCoreThread());
}
