#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux/AboutDialog.h"
#include "iptux/IptuxResource.h"

using namespace std;
using namespace iptux;

TEST(AboutDialog, aboutDialogNew) {
  gtk_init(nullptr, nullptr);
  iptux_register_resource();
  auto aboutDialog = aboutDialogNew();
  g_object_ref_sink(G_OBJECT(aboutDialog));
  g_object_unref(aboutDialog);
}
