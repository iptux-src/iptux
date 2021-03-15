#include "gtest/gtest.h"

#include "iptux/DialogGroup.h"
#include "iptux-core/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(DialogGroup, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  Application app(config);
  app.startup();
  app.activate();

  auto pal = make_shared<PalInfo>();
  pal->iconfile = g_strdup("pig");

  GroupInfo groupInfo(GROUP_BELONG_TYPE_SEGMENT, vector<PPalInfo>({pal}));

  DialogGroup* dialog = DialogGroup::GroupDialogEntry(&app, &groupInfo);
  delete dialog;
}
