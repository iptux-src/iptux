#include "gtest/gtest.h"

#include "iptux/Application.h"
#include "iptux/ShareFile.h"
#include "iptux/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(ShareFile, Constructor) {
  Application* app = CreateApplication();

  ShareFile* shareFile = shareFileNew(app);
  gtk_widget_destroy(GTK_WIDGET(shareFile));
  DestroyApplication(app);
}
