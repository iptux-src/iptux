#include "Application.h"
#include "UiModels.h"
#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux/MainWindow.h"
#include "iptux/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(MainWindow, GetTitle) {
  Application* app = CreateApplication();
  MainWindow mw(app, *app->getCoreThread());
  ASSERT_EQ(mw.getTitle(), "Iptux");
  DestroyApplication(app);
}

TEST(MainWindow, GetTitleWithBindIp) {
  auto config = newTestIptuxConfig();
  config->SetString("bind_ip", "127.0.0.1");
  Application* app = CreateApplication(config);
  MainWindow mw(app, *app->getCoreThread());
  ASSERT_EQ(mw.getTitle(), "Iptux - 127.0.0.1");
  DestroyApplication(app);
}

TEST(MainWindow, GetTitleWithBindIpAndPort) {
  auto config = newTestIptuxConfig();
  config->SetString("bind_ip", "127.0.0.1");
  config->SetInt("port", 3456);
  Application* app = CreateApplication(config);
  MainWindow mw(app, *app->getCoreThread());
  ASSERT_EQ(mw.getTitle(), "Iptux - 127.0.0.1:3456");
  DestroyApplication(app);
}

TEST(MainWindow, GetTitleWithPortOnly) {
  auto config = newTestIptuxConfig();
  config->SetInt("port", 3456);
  Application* app = CreateApplication(config);
  MainWindow mw(app, *app->getCoreThread());
  ASSERT_EQ(mw.getTitle(), "Iptux - :3456");
  DestroyApplication(app);
}

void do_action(MainWindow* w, const string& name, const string& val) {
  GActionMap* m = G_ACTION_MAP(w->getWindow());
  g_action_change_state(g_action_map_lookup_action(m, name.c_str()),
                        g_variant_new_string(val.c_str()));
}

TEST(MainWindow, Constructor) {
  Application* app = CreateApplication();

  MainWindow mw(app, *app->getCoreThread());
  mw.getWindow();

  do_action(&mw, "sort_by", "nickname");
  ASSERT_EQ(mw.sort_key(), PalTreeModelSortKey::NICKNAME);
  do_action(&mw, "sort_by", "username");
  ASSERT_EQ(mw.sort_key(), PalTreeModelSortKey::USERNAME);
  do_action(&mw, "sort_by", "ip");
  ASSERT_EQ(mw.sort_key(), PalTreeModelSortKey::IP);
  do_action(&mw, "sort_by", "host");
  ASSERT_EQ(mw.sort_key(), PalTreeModelSortKey::HOST);
  do_action(&mw, "sort_by", "last_activity");
  ASSERT_EQ(mw.sort_key(), PalTreeModelSortKey::LAST_ACTIVITY);

  do_action(&mw, "sort_type", "ascending");
  ASSERT_EQ(mw.sort_type(), GTK_SORT_ASCENDING);
  do_action(&mw, "sort_type", "descending");
  ASSERT_EQ(mw.sort_type(), GTK_SORT_DESCENDING);

  do_action(&mw, "info_style", "ip");
  ASSERT_EQ(mw.info_style(), GroupInfoStyle::IP);
  do_action(&mw, "info_style", "username");
  ASSERT_EQ(mw.info_style(), GroupInfoStyle::USERNAME);
  do_action(&mw, "info_style", "host");
  ASSERT_EQ(mw.info_style(), GroupInfoStyle::HOST);
  do_action(&mw, "info_style", "version");
  ASSERT_EQ(mw.info_style(), GroupInfoStyle::VERSION_NAME);
  do_action(&mw, "info_style", "last_activity");
  ASSERT_EQ(mw.info_style(), GroupInfoStyle::LAST_ACTIVITY);
  do_action(&mw, "info_style", "last_message");
  ASSERT_EQ(mw.info_style(), GroupInfoStyle::LAST_MESSAGE);

  DestroyApplication(app);
}
