#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux/Application.h"
#include "iptux/RevisePal.h"

using namespace std;
using namespace iptux;

TEST(RevisePal, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  Application app(config);
  app.startup();
  app.activate();

  PalInfo palInfo;

  RevisePal pal(app.getMainWindow(), &palInfo);
}

TEST(RevisePal, ReviseEntryDo) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  Application app(config);
  app.startup();
  app.activate();

  PalInfo palInfo;

  RevisePal::ReviseEntryDo(app.getMainWindow(), &palInfo, false);
}
