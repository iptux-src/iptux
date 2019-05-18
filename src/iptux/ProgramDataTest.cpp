#include "gtest/gtest.h"

#include <glib/gstdio.h>

#include "iptux/ProgramData.h"
#include "iptux/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(ProgramData, Constructor) {
  auto config = newTestIptuxConfig();
  ProgramData* core = new ProgramData(config);
  ASSERT_TRUE(core->IsSaveChatHistory());
  core->WriteProgData();
  delete core;

  g_unlink(config->getFileName().c_str());
}

TEST(ProgramData, WriteAndRead) {
  auto config = newTestIptuxConfig();
  ProgramData* core = new ProgramData(config);
  NetSegment netSegment;
  netSegment.startip = "1.2.3.4";
  netSegment.endip = "1.2.3.5";
  netSegment.description = "foobar";
  core->setNetSegments(vector<NetSegment>(1, netSegment));
  core->WriteProgData();
  delete core;


  auto config2 = make_shared<IptuxConfig>(config->getFileName());
  ProgramData* core2 = new ProgramData(config2);
  ASSERT_EQ(int(core2->getNetSegments().size()), 1);
  ASSERT_EQ(core2->getNetSegments()[0].startip, "1.2.3.4");
  ASSERT_EQ(core2->getNetSegments()[0].endip, "1.2.3.5");
  ASSERT_EQ(core2->getNetSegments()[0].description, "foobar");
  ASSERT_TRUE(core2->IsSaveChatHistory());
  ASSERT_FALSE(core2->IsUsingBlacklist());
  ASSERT_FALSE(core2->IsFilterFileShareRequest());
  delete core2;

  g_unlink(config->getFileName().c_str());
}

