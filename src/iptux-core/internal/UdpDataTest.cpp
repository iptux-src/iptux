#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux-core/internal/UdpData.h"
#include "iptux-utils/utils.h"

using namespace std;
using namespace iptux;

TEST(UdpData, getCommandNo) {
  auto coreThread = newCoreThread();
  UdpData data(*coreThread, inAddrFromString("127.0.0.1"), "", 0);
  ASSERT_EQ(data.getCommandNo(), 0);
}
