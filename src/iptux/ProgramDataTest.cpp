#include "gtest/gtest.h"

#include <glib/gstdio.h>

#include "iptux/ProgramData.h"
#include "iptux/TestHelper.h"

using namespace iptux;

TEST(ProgramData, Constructor) {
  auto config = newTestIptuxConfig();
  ProgramData* core = new ProgramData(*config);
  ASSERT_FALSE(core->IsSaveChatHistory());
  core->WriteProgData();
  delete core;

  g_unlink(config->getFileName().c_str());
}

TEST(ProgramData, WriteAndRead) {
  auto config = newTestIptuxConfig();
  ProgramData* core = new ProgramData(*config);
  NetSegment netSegment;
  netSegment.startip = "1.2.3.4";
  netSegment.endip = "1.2.3.5";
  netSegment.description = "foobar";
  core->netseg.push_back(netSegment);
  core->WriteProgData();
  delete core;


  auto config2 = new IptuxConfig(config->getFileName());
  ProgramData* core2 = new ProgramData(*config2);
  ASSERT_EQ(core2->netseg.size(), 1);
  ASSERT_EQ(core2->netseg[0].startip, "1.2.3.4");
  ASSERT_EQ(core2->netseg[0].endip, "1.2.3.5");
  ASSERT_EQ(core2->netseg[0].description, "foobar");
  delete core2;
  delete config2;

  g_unlink(config->getFileName().c_str());
}

