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

  GroupInfo groupInfo;

  DialogGroup* dialog = DialogGroup::GroupDialogEntry(&app, &groupInfo);
  delete dialog;
}
