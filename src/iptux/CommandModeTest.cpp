#include "gtest/gtest.h"

#include "CommandMode.h"

#include "iptux-core/ipmsg.h"

using namespace iptux;
using namespace std;

TEST(CommandMode, toString) {
  EXPECT_EQ(CommandMode(0).toString(), "unkown command mode: 0");
  EXPECT_EQ(CommandMode(1).toString(), "BR_ENTRY");
  EXPECT_EQ(CommandMode(2).toString(), "BR_EXIT");
  EXPECT_EQ(CommandMode(3).toString(), "ANSENTRY");
  EXPECT_EQ(CommandMode(4).toString(), "BR_ABSENCE");
}
