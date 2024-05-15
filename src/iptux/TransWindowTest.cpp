#include "gtest/gtest.h"

#include "iptux/TestHelper.h"
#include "iptux/TransWindow.h"

using namespace std;
using namespace iptux;

TEST(TransWindow, Constructor) {
  Application* app = CreateApplication();

  TransWindow* transWindow = trans_window_new(app, nullptr);
  gtk_widget_destroy(GTK_WIDGET(transWindow));

  DestroyApplication(app);
}
