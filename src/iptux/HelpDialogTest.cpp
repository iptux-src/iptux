#include "gtest/gtest.h"

#include "iptux/HelpDialog.h"
#include "iptux-core/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(HelpDialog, AboutEntry) {
  HelpDialog::AboutEntry(nullptr, false);
}
