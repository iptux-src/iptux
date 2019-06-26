#include "gtest/gtest.h"

#include "iptux-core/internal/UdpData.h"
#include "iptux-core/TestHelper.h"
#include "iptux-core/utils.h"

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
