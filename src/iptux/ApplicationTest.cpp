#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux/Application.h"

using namespace std;
using namespace iptux;

TEST(Application, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  Application app(config);
  app.startup();
  app.activate();
}
