#include "gtest/gtest.h"

#include "support.h"

using namespace iptux;
using namespace std;

TEST(Support, get_sys_broadcast_addr) {
  int udpSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  auto ips = get_sys_broadcast_addr(udpSock);
  ASSERT_LE(int(ips.size()), 2);
  shutdown(udpSock, SHUT_RDWR);
}
