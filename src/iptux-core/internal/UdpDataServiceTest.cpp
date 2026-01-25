#include "gtest/gtest.h"

#include <memory>

#include "gio/gio.h"
#include "iptux-core/TestHelper.h"
#include "iptux-core/internal/UdpDataService.h"
#include "iptux-utils/utils.h"

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

TEST(UdpDataService, process) {
  auto core = newCoreThread();
  auto service = make_unique<UdpDataService>(*core.get());
  GSocketAddress* addr = makeTestAddress("127.0.0.1", 1234);
  service->process(addr, "", 0, true);
  g_object_unref(addr);
}

TEST(UdpDataService, SomeoneEntry) {
  auto core = newCoreThread();
  auto service = make_unique<UdpDataService>(*core.get());
  const char* data = "iptux 0.8.0:1:lidaobing:lidaobing.lan:257:lidaobing";
  GSocketAddress* addr = makeTestAddress("127.0.0.1", 1234);
  service->process(addr, data, strlen(data), true);
  g_object_unref(addr);
}

TEST(UdpDataService, CreatePalInfo) {
  struct TestCase {
    const char* data;
    const char* expected;
  };

  TestCase testCases[] = {
      {
          .data = "1_iptux "
                  "0.8.0-b1:6:lidaobing:LIs-MacBook-Pro.local:259:"
                  "中\xe4\xb8\x00\x00"
                  "icon-tux.png\x00utf-8\x00",
          .expected = "PalInfo(IP=127.0.0.1,name=中��,segdes=,version=1_iptux "
                      "0.8.0-b1,user=lidaobing,host=LIs-MacBook-Pro.local,"
                      "group=,photo=(NULL),sign=(NULL),iconfile=icon-qq.png,"
                      "encode=utf-8,packetn=0,rpacketn=0,compatible=0,online=1,"
                      "changed=0,in_blacklist=0)",
      },
      {
          .data = "1_iptux "
                  "0.8.0-b1:6:中\xe4\xb8:LIs-MacBook-Pro.local:259:"
                  "中\xe4\xb8\x00\x00icon-tux.png\x00utf-8\x00",
          .expected =
              "PalInfo(IP=127.0.0.1,name=中��,segdes=,version=1_iptux "
              "0.8.0-b1,user=中��,host=LIs-MacBook-Pro.local,"
              "group=,photo=(NULL),sign=(NULL),iconfile=icon-qq.png,encode=utf-"
              "8,packetn=0,rpacketn=0,compatible=0,online=1,changed=0,in_"
              "blacklist=0)",
      },
  };

  auto core = newCoreThread();
  for (const auto& tc : testCases) {
    auto service = make_unique<UdpDataService>(*core.get());
    GSocketAddress* addr = makeTestAddress("127.0.0.1", 1234);
    auto udp = service->process(addr, tc.data, strlen(tc.data), false);
    g_object_unref(addr);
    auto pal = udp->CreatePalInfo();
    ASSERT_EQ(pal->toString(), tc.expected);
  }
}
