#include "gtest/gtest.h"

#include <memory>
#include <vector>

#include "iptux-core/Models.h"
#include "iptux-utils/utils.h"
#include "iptux/UiModels.h"

using namespace std;
using namespace iptux;

TEST(TransModel, transModelIsFinished) {
  TransModel* transModel = transModelNew();
  ASSERT_TRUE(transModelIsFinished(transModel));

  TransFileModel transFileModel;
  transFileModel.setTaskId(1);
  ASSERT_FALSE(transFileModel.isFinished());
  transModelUpdateFromTransFileModel(transModel, transFileModel);
  ASSERT_FALSE(transModelIsFinished(transModel));
  ASSERT_EQ(gtk_tree_model_iter_n_children(transModel, nullptr), 1);

  transFileModel.finish();
  ASSERT_TRUE(transFileModel.isFinished());
  transModelUpdateFromTransFileModel(transModel, transFileModel);
  ASSERT_TRUE(transModelIsFinished(transModel));

  TransFileModel transFileModel2;
  transFileModel2.setTaskId(2);
  ASSERT_FALSE(transFileModel2.isFinished());
  ASSERT_NE(transFileModel.getTaskId(), transFileModel2.getTaskId());
  transModelUpdateFromTransFileModel(transModel, transFileModel2);
  ASSERT_EQ(gtk_tree_model_iter_n_children(transModel, nullptr), 2);
  ASSERT_FALSE(transModelIsFinished(transModel));
}

TEST(GroupInfo, GetInfoAsMarkup) {
  PalInfo pal("127.0.0.1", 2425);
  pal.setName("palname");
  PalInfo me("127.0.0.2", 2425);
  PPalInfo cpal = make_shared<PalInfo>(pal);
  CPPalInfo cme = make_shared<PalInfo>(me);
  GroupInfo gi(cpal, cme, nullptr);
  ASSERT_EQ(gi.GetInfoAsMarkup(GroupInfoStyle::IP), "palname\n127.0.0.1");
  ASSERT_EQ(gi.GetInfoAsMarkup(GroupInfoStyle::IP_PORT),
            "palname\n127.0.0.1:2425");

  MsgPara msg(cpal);
  gi.addMsgPara(msg);
  ASSERT_EQ(gi.GetInfoAsMarkup(GroupInfoStyle::IP),
            "palname <span foreground=\"red\">(1)</span>\n127.0.0.1");

  vector<PPalInfo> pals;
  pals.push_back(cpal);
  GroupInfo gi2(GROUP_BELONG_TYPE_SEGMENT, pals, cme, "group_name", nullptr);
  ASSERT_EQ(gi2.GetInfoAsMarkup(GroupInfoStyle::IP), "group_name");
}

TEST(GroupInfo, GetHintAsMarkup) {
  PalInfo pal("127.0.0.1", 2425);
  pal.setVersion("1_iptux");
  pal.setName("palname");
  PalInfo me("127.0.0.2", 2425);
  PPalInfo cpal = make_shared<PalInfo>(pal);
  CPPalInfo cme = make_shared<PalInfo>(me);
  GroupInfo gi(cpal, cme, nullptr);
  ASSERT_EQ(gi.GetHintAsMarkup(),
            "Version: 1_iptux\nNickname: palname\nUser: \nHost: \nAddress: "
            "127.0.0.1\nCompatibility: Microsoft\nSystem coding: ");

  cpal->sign = strdup("hello");
  ASSERT_EQ(gi.GetHintAsMarkup(),
            "Version: 1_iptux\nNickname: palname\nUser: \nHost: \nAddress: "
            "127.0.0.1\nCompatibility: Microsoft\nSystem coding: "
            "\nSignature:\n<span foreground=\"#00FF00\" font_style=\"italic\" "
            "size=\"smaller\">hello</span>");
}
