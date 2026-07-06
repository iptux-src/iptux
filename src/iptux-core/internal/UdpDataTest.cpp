#include "gtest/gtest.h"

#include "gio/gio.h"
#include "iptux-core/TestHelper.h"
#include "iptux-core/internal/UdpData.h"

using namespace std;
using namespace iptux;

namespace {

GSocketAddress* makeTestAddress(const char* ip, guint16 port) {
  GInetAddress* inet_addr = g_inet_address_new_from_string(ip);
  GSocketAddress* addr = g_inet_socket_address_new(inet_addr, port);
  g_object_unref(inet_addr);
  return addr;
}

}  // namespace

TEST(UdpData, getCommandNo) {
  auto coreThread = newCoreThread();
  GSocketAddress* addr = makeTestAddress("127.0.0.1", 1234);
  UdpData data(*coreThread, addr, "", 0);
  g_object_unref(addr);
  ASSERT_EQ(data.getCommandNo(), 0U);
}
