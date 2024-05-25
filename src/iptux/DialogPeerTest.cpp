#include "Application.h"
#include "gtest/gtest.h"

#include "iptux-utils/TestHelper.h"
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
  grpinf->buffer = gtk_text_buffer_new(NULL);
  DialogPeer* dlgpr = new DialogPeer(app, grpinf);

  auto clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  gtk_clipboard_set_text(clipboard, "hello world", -1);
  do_action(dlgpr, "paste");

  GError* error = NULL;
  auto pixbuf =
      gdk_pixbuf_new_from_file(testDataPath("iptux.png").c_str(), &error);
  if (error != nullptr) {
    ASSERT_TRUE(false) << error->message;
    g_error_free(error);
  }
  gtk_clipboard_set_image(clipboard, pixbuf);
  do_action(dlgpr, "paste");
  g_object_unref(pixbuf);

  MsgPara msg(pal);
  msg.dtlist.push_back(ChipData("helloworld"));

  grpinf->addMsgPara(msg);

  msg = MsgPara(pal);
  msg.dtlist.push_back(
      ChipData(MessageContentType::PICTURE, testDataPath("iptux.png")));
  grpinf->addMsgPara(msg);

  DestroyApplication(app);
}
