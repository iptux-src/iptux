#include "gtest/gtest.h"

#include "iptux-utils/TestHelper.h"
#include <clocale>
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

static string igtk_text_get_all_text(GtkTextBuffer* buffer) {
  GtkTextIter start, end;
  gtk_text_buffer_get_start_iter(buffer, &start);
  gtk_text_buffer_get_end_iter(buffer, &end);
  char* s = gtk_text_buffer_get_slice(buffer, &start, &end, true);
  string ret(s);
  g_free(s);
  return ret;
}

TEST(GroupInfo, addMsgPara) {
  setlocale(LC_ALL, "C");
  PalInfo pal("127.0.0.1", 2425);
  pal.setVersion("1_iptux");
  pal.setName("palname");
  PalInfo me("127.0.0.2", 2425);
  PPalInfo cpal = make_shared<PalInfo>(pal);
  CPPalInfo cme = make_shared<PalInfo>(me);
  GroupInfo gi(cpal, cme, nullptr);
  gi.buffer = gtk_text_buffer_new(nullptr);

  MsgPara msg(cpal);
  msg.dtlist.push_back(ChipData("helloworld"));

  time_t now = 1716533706;
  setenv("TZ", "GMT", 1);
  tzset();

  gi._addMsgPara(msg, now);
  ASSERT_EQ(igtk_text_get_all_text(gi.buffer),
            "(06:55:06) palname:\nhelloworld\n");

  msg = MsgPara(cpal);
  msg.dtlist.push_back(
      ChipData(MessageContentType::PICTURE, testDataPath("iptux.png")));
  gi._addMsgPara(msg, now + 1);
  ASSERT_EQ(
      igtk_text_get_all_text(gi.buffer),
      "(06:55:06) palname:\nhelloworld\n(06:55:07) palname:\n\xEF\xBF\xBC\n");
}

TEST(GroupInfo, genMsgParaFromInput) {
  PalInfo pal("127.0.0.1", 2425);
  pal.setVersion("1_iptux");
  pal.setName("palname");
  PalInfo me("127.0.0.2", 2425);
  PPalInfo cpal = make_shared<PalInfo>(pal);
  CPPalInfo cme = make_shared<PalInfo>(me);
  GroupInfo gi(cpal, cme, nullptr);

  auto para = gi.genMsgParaFromInput();
  ASSERT_EQ(para->dtlist.size(), 0);

  GtkTextIter end;
  gtk_text_buffer_get_end_iter(gi.getInputBuffer(), &end);
  gtk_text_buffer_insert(gi.getInputBuffer(), &end, "helloworld", -1);
  para = gi.genMsgParaFromInput();
  ASSERT_EQ(para->dtlist.size(), 1);
  ASSERT_EQ(para->dtlist[0].type, MessageContentType::STRING);
  ASSERT_EQ(para->dtlist[0].data, "helloworld");
}
