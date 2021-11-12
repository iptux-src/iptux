#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux-core/internal/UdpData.h"
#include "iptux-utils/utils.h"

using namespace std;
using namespace iptux;

TEST(UdpData, getCommandNo) {
  ASSERT_EQ(UdpData("", "127.0.0.1").getCommandNo(), 0);
}
