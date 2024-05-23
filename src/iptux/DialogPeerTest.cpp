#include "Application.h"
#include "gtest/gtest.h"

#include "iptux/DialogPeer.h"
#include "iptux/TestHelper.h"
#include "iptux/UiCoreThread.h"

using namespace std;
using namespace iptux;

static void do_action(DialogPeer* w, const char* name) {
  GActionMap* m = G_ACTION_MAP(w->getWindow());
  g_action_activate(g_action_map_lookup_action(m, name), NULL);
}

TEST(DialogPeer, Constructor) {
  Application* app = CreateApplication();

  PPalInfo pal = make_shared<PalInfo>("127.0.0.1", 2425);
  app->getCoreThread()->AttachPalToList(pal);

  GroupInfo* grpinf = app->getCoreThread()->GetPalRegularItem(pal.get());
  DialogPeer* dlgpr = new DialogPeer(app, grpinf);
  do_action(dlgpr, "paste");

  auto clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  gtk_clipboard_set_text(clipboard, "hello world", -1);
  do_action(dlgpr, "paste");

  DestroyApplication(app);
}
