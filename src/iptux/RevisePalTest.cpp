#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux/Application.h"
#include "iptux/RevisePal.h"
#include "iptux/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(RevisePal, Constructor) {
  Application* app = CreateApplication();

  PalInfo palInfo;

  RevisePal pal(app, nullptr, &palInfo);
  DestroyApplication(app);
}

TEST(RevisePal, ReviseEntryDo) {
  Application* app = CreateApplication();

  PalInfo palInfo;

  RevisePal::ReviseEntryDo(app, nullptr, &palInfo, false);
  DestroyApplication(app);
}
