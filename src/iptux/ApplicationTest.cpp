#include "gtest/gtest.h"

#include "iptux/Application.h"
#include "iptux/TestHelper.h"

using namespace std;
using namespace iptux;

void do_action(Application* app, const string& name) {
  GActionMap* m = G_ACTION_MAP(app->getApp());
  g_action_activate(g_action_map_lookup_action(m, name.c_str()), NULL);
}


TEST(Application, Constructor) {
  Application* app = CreateApplication();
  do_action(app, "help.whats_new");
  DestroyApplication(app);
}
