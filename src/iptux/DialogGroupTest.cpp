#include "Application.h"
#include "gtest/gtest.h"

#include "iptux/DialogGroup.h"
#include "iptux/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(DialogGroup, Constructor) {
  Application* app = CreateApplication();

  auto pal = make_shared<PalInfo>();
  pal->iconfile = g_strdup("pig");

  GroupInfo groupInfo(GROUP_BELONG_TYPE_SEGMENT, vector<PPalInfo>({pal}),
                      app->getMe(), "groupname", nullptr);

  DialogGroup* dialog = DialogGroup::GroupDialogEntry(app, &groupInfo);
  delete dialog;

  DestroyApplication(app);
}
