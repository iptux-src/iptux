#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux/DialogPeer.h"
#include "iptux/UiCoreThread.h"

using namespace std;
using namespace iptux;

TEST(DialogPeer, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  Application app(config);
  app.startup();
  app.activate();

  PPalInfo pal = make_shared<PalInfo>();
  app.getCoreThread()->AttachPalToList(pal);

  GroupInfo* grpinf = app.getCoreThread()->GetPalRegularItem(pal.get());
  DialogPeer::PeerDialogEntry(&app, grpinf);
}
