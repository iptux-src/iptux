#include "gtest/gtest.h"

#include "iptux/Models.h"

using namespace std;
using namespace iptux;

TEST(PalInfo, GetKey) {
  PalInfo info;
  ASSERT_EQ(info.GetKey().ToString(), "0.0.0.0:2425");
}

TEST(NetSegment, ContainIP) {
  NetSegment netSegment;
  netSegment.startip = "1.2.3.4";
  netSegment.endip = "1.2.4.5";

  vector<string> ips = {
    "1.2.3.4",
    "1.2.4.5",
    "1.2.3.255",
    "1.2.4.0",
    "1.2.3.5",
    "1.2.4.4"
  };

  for (const string& ip: ips) {
    in_addr_t ip1;
    ASSERT_EQ(inet_pton(AF_INET, ip.c_str(), &ip1), 1) << ip;
    ASSERT_TRUE(netSegment.ContainIP(ip1));
  }

  vector<string> ips2 = {
    "1.2.3.3",
    "1.2.4.6",
    "0.0.0.0",
    "100.100.100.100",
  };
  for (const string& ip: ips2) {
    in_addr_t ip1;
    ASSERT_EQ(inet_pton(AF_INET, ip.c_str(), &ip1), 1) << ip;
    ASSERT_FALSE(netSegment.ContainIP(ip1));
  }
}

TEST(ChipData, ToString) {
  EXPECT_EQ(ChipData().ToString(), "ChipData(MessageContentType::STRING, )");
}

