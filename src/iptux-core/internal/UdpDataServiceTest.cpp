#include "gtest/gtest.h"

#include <memory>

#include "iptux-core/TestHelper.h"
#include "iptux-core/internal/UdpDataService.h"
#include "iptux-utils/utils.h"

using namespace std;
using namespace iptux;

TEST(UdpDataService, process) {
  auto core = newCoreThread();
  auto service = make_unique<UdpDataService>(*core.get());
  service->process(inAddrFromString("127.0.0.1"), 1234, "", 0, true);
}

TEST(UdpDataService, SomeoneEntry) {
  auto core = newCoreThread();
  auto service = make_unique<UdpDataService>(*core.get());
  const char* data = "iptux 0.8.0:1:lidaobing:lidaobing.lan:257:lidaobing";
  service->process(inAddrFromString("127.0.0.1"), 1234, data, strlen(data),
                   true);
}

TEST(UdpDataService, CreatePalInfo) {
  auto core = newCoreThread();
  {
    const char* data =
        "1_iptux "
        "0.8.0-b1:6:lidaobing:LIs-MacBook-Pro.local:259:中\xe4\xb8\x00\x00icon-"
        "tux.png\x00utf-8\x00";
    auto service = make_unique<UdpDataService>(*core.get());
    auto udp = service->process(inAddrFromString("127.0.0.1"), 1234, data,
                                strlen(data), false);
    auto pal = udp->CreatePalInfo();
    ASSERT_EQ(pal->toString(),
              "PalInfo(IP=127.0.0.1,name=中��,segdes=,version=1_iptux "
              "0.8.0-b1,user=lidaobing,host=LIs-MacBook-Pro.local,"
              "group=,photo=(NULL),sign=(NULL),iconfile=icon-qq.png,"
              "encode=utf-8,packetn=0,rpacketn=0,compatible=0,online=1,"
              "changed=0,in_blacklist=0)");
  }
  {
    const char* data =
        "1_iptux "
        "0.8.0-b1:6:中\xe4\xb8:LIs-MacBook-Pro.local:259:"
        "中\xe4\xb8\x00\x00icon-tux.png\x00utf-8\x00";
    auto service = make_unique<UdpDataService>(*core.get());
    auto udp = service->process(inAddrFromString("127.0.0.1"), 1234, data,
                                strlen(data), false);
    auto pal = udp->CreatePalInfo();
    ASSERT_EQ(pal->toString(),
              "PalInfo(IP=127.0.0.1,name=中��,segdes=,version=1_iptux "
              "0.8.0-b1,user=中��,host=LIs-MacBook-Pro.local,"
              "group=,photo=(NULL),sign=(NULL),iconfile=icon-qq.png,encode=utf-"
              "8,packetn=0,rpacketn=0,compatible=0,online=1,changed=0,in_"
              "blacklist=0)");
  }
}
