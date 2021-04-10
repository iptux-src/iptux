#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux/AboutDialog.h"

using namespace std;
using namespace iptux;

TEST(AboutDialog, aboutDialogNew) {
  auto aboutDialog = aboutDialogNew();
  g_object_unref(aboutDialog);
}
