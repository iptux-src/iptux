#include "gtest/gtest.h"

#include "iptux-core/internal/UdpData.h"
#include "iptux-core/TestHelper.h"
#include "iptux-utils/utils.h"

using namespace std;
using namespace iptux;

TEST(UdpData, UdpDataEntry) {
  auto config = newTestIptuxConfig();
  auto core = make_shared<ProgramData>(config);
  CoreThread* thread = new CoreThread(core);
  UdpData::UdpDataEntry(*thread, inAddrFromString("127.0.0.1"), 1234, "", 0);
  delete thread;
}

TEST(UdpData, SomeoneEntry) {
  auto config = newTestIptuxConfig();
  auto core = make_shared<ProgramData>(config);
  CoreThread* thread = new CoreThread(core);
  const char* data = "iptux 0.8.0:1:lidaobing:lidaobing.lan:257:lidaobing";
  UdpData::UdpDataEntry(*thread, inAddrFromString("127.0.0.1"), 1234, data, strlen(data));
  delete thread;
}

TEST(UdpData, CreatePalInfo) {
  auto core = newCoreThread();
  const char* data = "1_iptux 0.8.0-b1:6:lidaobing:LIs-MacBook-Pro.local:259:中\xe4\xb8\x00\x00icon-tux.png\x00utf-8\x00";
  auto udp = UdpData::UdpDataEntry(*core.get(), inAddrFromString("127.0.0.1"), 1234, data, strlen(data), false);
  auto pal = udp->CreatePalInfo();
  ASSERT_EQ(pal->getName(), "中��");
}
