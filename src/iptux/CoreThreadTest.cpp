#include "gtest/gtest.h"

#include <thread>

#include "iptux/CoreThread.h"
#include "iptux/TestHelper.h"
#include "iptux/utils.h"
#include "iptux/Exception.h"
#include "iptux/output.h"

using namespace std;
using namespace iptux;

TEST(CoreThread, Constructor) {
  auto config = newTestIptuxConfig();
  auto core = make_shared<ProgramData>(config);
  core->sign = "abc";
  CoreThread* thread = new CoreThread(core);
  thread->start();
  thread->stop();
  delete thread;
}

TEST(CoreThread, IsBlocked) {
  auto config = newTestIptuxConfig();
  auto core = make_shared<ProgramData>(config);
  core->sign = "abc";
  CoreThread* thread = new CoreThread(core);
  EXPECT_FALSE(thread->IsBlocked(stringToInAddr("1.2.3.4")));
  thread->AddBlockIp(stringToInAddr("1.2.3.4"));
  EXPECT_FALSE(thread->IsBlocked(stringToInAddr("1.2.3.4")));
  core->SetUsingBlacklist(true);
  EXPECT_TRUE(thread->IsBlocked(stringToInAddr("1.2.3.4")));
  core->SetUsingBlacklist(false);
  EXPECT_FALSE(thread->IsBlocked(stringToInAddr("1.2.3.4")));
  delete thread;
}

TEST(CoreThread, GetPalList) {
  auto config = newTestIptuxConfig();
  auto core = make_shared<ProgramData>(config);
  core->sign = "abc";
  CoreThread* thread = new CoreThread(core);
  EXPECT_EQ(int(thread->GetPalList().size()), 0);
  PPalInfo pal = make_shared<PalInfo>();
  thread->AttachPalToList(pal);
  EXPECT_EQ(int(thread->GetPalList().size()), 1);
  delete thread;
}

TEST(CoreThread, SendMessage) {
  auto config = newTestIptuxConfig();
  auto core = make_shared<ProgramData>(config);
  core->sign = "abc";
  CoreThread* thread = new CoreThread(core);
  auto pal = make_shared<PalInfo>();
  try {
    thread->SendMessage(pal, "hello world");
    EXPECT_TRUE(false);
  } catch(Exception& e) {
    EXPECT_EQ(e.getErrorCode(), ErrorCode::PAL_KEY_NOT_EXIST);
  }

  thread->AttachPalToList(pal);
  EXPECT_TRUE(thread->SendMessage(pal, "hello world"));
  delete thread;
}

TEST(CoreThread, SendMessage_ChipData) {
  auto config = newTestIptuxConfig();
  auto core = make_shared<ProgramData>(config);
  core->sign = "abc";
  CoreThread* thread = new CoreThread(core);
  auto pal = make_shared<PalInfo>();
  thread->AttachPalToList(pal);
  ChipData chipData;
  chipData.data = "hello world";
  EXPECT_TRUE(thread->SendMessage(pal, chipData));
  delete thread;
}

TEST(CoreThread, SendMsgPara) {
  auto config = newTestIptuxConfig();
  auto core = make_shared<ProgramData>(config);
  core->sign = "abc";
  CoreThread* thread = new CoreThread(core);
  PPalInfo pal = make_shared<PalInfo>();
  thread->AttachPalToList(pal);
  ChipData chipData;
  chipData.data = "hello world";
  MsgPara para;
  para.pal = pal;
  para.dtlist.push_back(move(chipData));
  EXPECT_TRUE(thread->SendMsgPara(para));
  delete thread;
}

// TEST(CoreThread, AsyncSendMsgPara) {
//   auto config = newTestIptuxConfig();
//   ProgramData* core = new ProgramData(*config);
//   core->sign = "abc";
//   CoreThread* thread = new CoreThread(*core);
//   PalInfo pal;
//   ChipData chipData;
//   chipData.data = "hello world";
//   MsgPara para;
//   para.pal = &pal;
//   para.dtlist.push_back(move(chipData));
//   thread->AsyncSendMsgPara(move(para));
//   delete thread;
//   delete core;
// }

TEST(CoreThread, SendAskShared) {
  auto config = newTestIptuxConfig();
  auto core = make_shared<ProgramData>(config);
  core->sign = "abc";
  CoreThread* thread = new CoreThread(core);
  auto pal = make_shared<PalInfo>();
  thread->SendAskShared(pal);
  delete thread;
}

TEST(CoreThread, FullCase) {
  using namespace std::chrono_literals;
  auto oldLogLevel = Log::getLogLevel();
  Log::setLogLevel(LogLevel::DEBUG);
  auto config1 = IptuxConfig::newFromString("{\"bind_ip\": \"127.0.0.1\"}");
  auto thread1 = new CoreThread(make_shared<ProgramData>(config1));
  thread1->start();
  auto config2 = IptuxConfig::newFromString("{\"bind_ip\": \"127.0.0.2\"}");
  auto thread2 = new CoreThread(make_shared<ProgramData>(config2));
  thread2->start();
  thread1->SendDetectPacket("127.0.0.2");
  while(thread2->GetOnlineCount() != 1) {
    thread2->Lock();
    cout << thread2->GetOnlineCount() << endl;
    thread2->Unlock();
    this_thread::sleep_for(10ms);
  }

  EXPECT_TRUE(thread2->GetPal("127.0.0.1"));
  EXPECT_EQ(thread1->GetOnlineCount(), 1);
  EXPECT_TRUE(thread1->GetPal("127.0.0.2"));

  Log::setLogLevel(oldLogLevel);
}
