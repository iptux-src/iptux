#include "gtest/gtest.h"

#include "support.h"

using namespace iptux;
using namespace std;

TEST(Support, get_sys_broadcast_addr) {
  GError* error = nullptr;
  GSocket* udpSock = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_DATAGRAM,
                                  G_SOCKET_PROTOCOL_UDP, &error);
  ASSERT_EQ(error, nullptr);
  auto ips = get_sys_broadcast_addr(udpSock);
  ASSERT_GE(int(ips.size()), 2);
  g_object_unref(udpSock);
}
