#include "gtest/gtest.h"

#include "iptux/DialogPeer.h"
#include "iptux-core/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(DialogPeer, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  Application app(config);
  app.startup();
  app.activate();

  GroupInfo groupInfo;

  DialogPeer* dialog = PeerDialogEntry(&app, &GroupInfo, app->getProgramData());
  delete dialog;
}
