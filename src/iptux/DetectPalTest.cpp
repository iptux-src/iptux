#include "gtest/gtest.h"

#include "iptux/DetectPal.h"
#include "iptux/TestHelper.h"
#include "iptux-core/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(DetectPal, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  Application app(config);
  app.startup();
  app.activate();

  auto builder = newTestGtkBuilder();

  DetectPal pal(&app, builder, nullptr);
}
