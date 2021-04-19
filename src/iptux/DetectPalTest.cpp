#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux/DetectPal.h"
#include "iptux/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(DetectPal, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  Application app(config);
  app.startup();
  app.activate();

  DetectPal pal(&app, nullptr);
}
