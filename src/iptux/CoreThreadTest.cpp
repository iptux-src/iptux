#include "gtest/gtest.h"

#include "iptux/CoreThread.h"
#include "iptux/TestHelper.h"
#include "iptux/utils.h"

using namespace iptux;

TEST(CoreThread, Constructor) {
  auto config = newTestIptuxConfig();
  ProgramData* core = new ProgramData(*config);
  core->sign = "abc";
  CoreThread* thread = new CoreThread(*core);
  thread->start();
  thread->stop();
  delete thread;
  delete core;
}

TEST(CoreThread, IsBlocked) {
  auto config = newTestIptuxConfig();
  ProgramData* core = new ProgramData(*config);
  core->sign = "abc";
  CoreThread* thread = new CoreThread(*core);
  EXPECT_FALSE(thread->IsBlocked(stringToInAddr("1.2.3.4")));
  thread->AddBlockIp(stringToInAddr("1.2.3.4"));
  EXPECT_FALSE(thread->IsBlocked(stringToInAddr("1.2.3.4")));
  core->SetUsingBlacklist(true);
  EXPECT_TRUE(thread->IsBlocked(stringToInAddr("1.2.3.4")));
  core->SetUsingBlacklist(false);
  EXPECT_FALSE(thread->IsBlocked(stringToInAddr("1.2.3.4")));
  delete thread;
  delete core;
}

TEST(CoreThread, GetPalList) {
  auto config = newTestIptuxConfig();
  ProgramData* core = new ProgramData(*config);
  core->sign = "abc";
  CoreThread* thread = new CoreThread(*core);
  EXPECT_EQ(thread->GetPalList(), nullptr);
  PalInfo pal;
  thread->AttachPalToList(&pal);
  EXPECT_NE(thread->GetPalList(), nullptr);
  delete thread;
  delete core;
}

TEST(CoreThread, SendMessage) {
  auto config = newTestIptuxConfig();
  ProgramData* core = new ProgramData(*config);
  core->sign = "abc";
  CoreThread* thread = new CoreThread(*core);
  PalInfo pal;
  thread->SendMessage(pal, "hello world");
  delete thread;
  delete core;
}
