#include "gtest/gtest.h"

#include "iptux/TransWindow.h"
#include "iptux-core/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(TransWindow, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  Application app(config);
  app.startup();
  app.activate();

  TransWindow* transWindow = trans_window_new(&app, nullptr);
  gtk_widget_destroy(GTK_WIDGET(transWindow));
}
