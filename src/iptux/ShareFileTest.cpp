#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux/Application.h"
#include "iptux/MainWindow.h"
#include "iptux/ShareFile.h"

using namespace std;
using namespace iptux;

TEST(ShareFile, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  Application app(config);
  app.startup();
  app.activate();

  ShareFile* shareFile =
      share_file_new(GTK_WINDOW(app.getMainWindow()->getWindow()));
  gtk_widget_destroy(GTK_WIDGET(shareFile));
}
