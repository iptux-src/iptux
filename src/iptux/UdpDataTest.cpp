#include "gtest/gtest.h"

#include "iptux/UdpData.h"
#include "iptux/TestHelper.h"
#include "iptux/utils.h"

using namespace iptux;

TEST(UdpData, UdpDataEntry) {
  auto config = newTestIptuxConfig();
  ProgramData* core = new ProgramData(*config);
  CoreThread* thread = new CoreThread(*core);
  UdpData::UdpDataEntry(*thread, stringToInAddr("127.0.0.1"), 1234, "", 0);
  delete thread;
  delete core;
}

TEST(UdpData, SomeoneEntry) {
  auto config = newTestIptuxConfig();
  ProgramData* core = new ProgramData(*config);
  CoreThread* thread = new CoreThread(*core);
  const char* data = "iptux 0.8.0:1:lidaobing:lidaobing.lan:257:lidaobing";
  UdpData::UdpDataEntry(*thread, stringToInAddr("127.0.0.1"), 1234, data, strlen(data));
  delete thread;
  delete core;
}
