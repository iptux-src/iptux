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
    this_thread::sleep_for(10ms);
  }

  EXPECT_TRUE(thread2->GetPal("127.0.0.1"));
  EXPECT_EQ(thread1->GetOnlineCount(), 1);
  EXPECT_TRUE(thread1->GetPal("127.0.0.2"));

  vector<shared_ptr<const Event>> thread2Events;
  thread2->registerCallback([&](shared_ptr<const Event> event) { thread2Events.emplace_back(event); });

  auto pal2InThread1 = thread1->GetPal("127.0.0.2");
  auto pal1InThread2 = thread2->GetPal("127.0.0.1");
  thread1->SendMessage(pal2InThread1, "hello world");
  while(thread2Events.size() != 1) {
    this_thread::sleep_for(10ms);
  }
  auto event = thread2Events[0];
  EXPECT_EQ(event->getType(), EventType::NEW_MESSAGE);
  auto event2 = (NewMessageEvent*)(event.get());
  EXPECT_EQ(event2->getMsgPara().dtlist[0].ToString(), "ChipData(MessageContentType::STRING, hello world)");

  thread1->SendExit(pal2InThread1);
  while(thread2->GetOnlineCount() != 0) {
    this_thread::sleep_for(10ms);
  }

  thread1->SendDetectPacket("127.0.0.2");
  while(thread2->GetOnlineCount() != 1) {
    this_thread::sleep_for(10ms);
  }

  Log::setLogLevel(oldLogLevel);
}
