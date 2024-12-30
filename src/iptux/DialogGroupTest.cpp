#include "Application.h"
#include "gtest/gtest.h"

#include "iptux/DialogGroup.h"
#include "iptux/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(DialogGroup, Constructor) {
  Application* app = CreateApplication();

  auto pal = make_shared<PalInfo>("127.0.0.1", 2425);
  pal->set_icon_file("pig");

  GroupInfo groupInfo(IPTUX_GROUP_BELONG_SEGMENT, vector<PPalInfo>({pal}),
                      app->getMe(), "groupname", nullptr);

  DialogGroup* dialog = DialogGroup::GroupDialogEntry(app, &groupInfo);
  delete dialog;

  DestroyApplication(app);
}
