#include "gtest/gtest.h"

#include "iptux/Application.h"
#include "iptux/TestHelper.h"
#include "iptux/UiCoreThread.h"

using namespace std;
using namespace iptux;

void do_action(Application* app, const string& name) {
  GActionMap* m = G_ACTION_MAP(app->getApp());
  g_action_activate(g_action_map_lookup_action(m, name.c_str()), NULL);
}

TEST(Application, Constructor) {
  Application* app = CreateApplication();
  do_action(app, "help.whats_new");
  do_action(app, "tools.open_chat_log");
  do_action(app, "tools.open_system_log");

  PPalInfo pal = make_shared<PalInfo>("127.0.0.1", 2425);
  app->getCoreThread()->AttachPalToList(pal);
  app->_ForTestProcessEvents();
  app->_ForTestProcessEvents();
  DestroyApplication(app);
}
