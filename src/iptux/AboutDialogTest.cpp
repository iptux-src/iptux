#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux/AboutDialog.h"

using namespace std;
using namespace iptux;

TEST(AboutDialog, aboutDialogNew) {
  gtk_init(nullptr, nullptr);
  auto aboutDialog = aboutDialogNew();
  g_object_ref_sink(G_OBJECT(aboutDialog));
  g_object_unref(aboutDialog);
}
