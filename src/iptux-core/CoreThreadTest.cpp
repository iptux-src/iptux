#include "gtest/gtest.h"

#include <thread>
#include <fstream>

#include "iptux-core/CoreThread.h"
#include "iptux-core/Exception.h"
#include "iptux-core/internal/support.h"
#include "iptux-core/internal/ipmsg.h"
#include "iptux-core/TestHelper.h"
#include "iptux-utils/utils.h"
#include "iptux-utils/output.h"

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
  EXPECT_FALSE(thread->IsBlocked(inAddrFromString("1.2.3.4")));
  thread->AddBlockIp(inAddrFromString("1.2.3.4"));
  EXPECT_FALSE(thread->IsBlocked(inAddrFromString("1.2.3.4")));
  core->SetUsingBlacklist(true);
  EXPECT_TRUE(thread->IsBlocked(inAddrFromString("1.2.3.4")));
  core->SetUsingBlacklist(false);
  EXPECT_FALSE(thread->IsBlocked(inAddrFromString("1.2.3.4")));
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
    EXPECT_EQ(e.getErrorCode(), PAL_KEY_NOT_EXIST);
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
  Log::setLogLevel(LogLevel::INFO);
  auto config1 = IptuxConfig::newFromString("{}");
  config1->SetString("bind_ip", "127.0.0.1");
  auto config2 = IptuxConfig::newFromString(
    "{"
      "\"bind_ip\": \"127.0.0.2\","
      "\"access_shared_limit\": \"qwert\","
      "\"personal_sign\": \"smartboy\""
    "}");
  auto threads = initAndConnnectThreadsFromConfig(config1, config2);
  auto thread1 = get<0>(threads);
  auto thread2 = get<1>(threads);

  EXPECT_TRUE(thread2->GetPal("127.0.0.1"));
  EXPECT_EQ(thread1->GetOnlineCount(), 1);
  EXPECT_TRUE(thread1->GetPal("127.0.0.2"));

  vector<shared_ptr<const Event>> thread2Events;
  thread2->registerCallback([&](shared_ptr<const Event> event) { thread2Events.emplace_back(event); });

  auto pal2InThread1 = thread1->GetPal("127.0.0.2");
  auto pal1InThread2 = thread2->GetPal("127.0.0.1");

  // send message
  thread1->SendMessage(pal2InThread1, "hello world");
  while(thread2Events.size() != 1) {
    this_thread::sleep_for(10ms);
  }
  auto event = thread2Events[0];
  EXPECT_EQ(event->getType(), EventType::NEW_MESSAGE);
  auto event2 = (NewMessageEvent*)(event.get());
  EXPECT_EQ(event2->getMsgPara().dtlist[0].ToString(), "ChipData(MessageContentType::STRING, hello world)");

  // send my icon
  ifstream ifs(testDataPath("iptux.png"));
  thread1->SendMyIcon(pal2InThread1, ifs);
  while(thread2Events.size() != 2) {
    this_thread::sleep_for(10ms);
  }
  {
    auto event = thread2Events[1];
    EXPECT_EQ(event->getType(), EventType::ICON_UPDATE);
    auto event2 = (IconUpdateEvent*)(event.get());
    EXPECT_EQ(event2->GetPalKey().ToString(), "127.0.0.1:2425");
  }

  // send ask shared
  thread1->SendAskShared(pal2InThread1);

  // send picture
  ChipData chipData;
  chipData.type = MessageContentType::PICTURE;
  chipData.data = testDataPath("iptux.png");
  chipData.SetDeleteFileAfterSent(false);
  thread1->SendMessage(pal2InThread1, chipData);
  // WARNING: does not work as expected, the message will be sent from 127.0.0.2(expect 127.0.0.1)
  // while(thread2Events.size() != 2) {
  //   this_thread::sleep_for(10ms);
  // }

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

TEST(CoreThread, FullCase_ShareWithPassword) {
  auto oldLogLevel = Log::getLogLevel();
  Log::setLogLevel(LogLevel::INFO);
  auto config1 = IptuxConfig::newFromString("{}");
  config1->SetString("bind_ip", "127.0.0.3");
  auto config2 = IptuxConfig::newFromString("{}");
  config2->SetString("bind_ip", "127.0.0.4");
  config2->SetString("access_shared_limit", "qwert");
  auto threads = initAndConnnectThreadsFromConfig(config1, config2);
  auto thread1 = get<0>(threads);
  auto thread2 = get<1>(threads);
  thread1->SendAskShared(thread1->GetPal("127.0.0.4"));
  Log::setLogLevel(oldLogLevel);
}

TEST(CoreThread, PrivateFiles) {
  auto thread = newCoreThreadOnIp("127.0.0.1");
  auto file = make_shared<FileInfo>();
  file->fileid = MAX_SHAREDFILE;
  file->filepath = g_strdup("hello");
  thread->AddPrivateFile(file);

  auto file2 = thread->GetPrivateFileById(file->fileid);
  EXPECT_STREQ(file2->filepath, "hello");

  EXPECT_TRUE(thread->DelPrivateFile(file->fileid));
  EXPECT_FALSE(thread->DelPrivateFile(file->fileid));

  EXPECT_FALSE(thread->GetPrivateFileById(file->fileid));

  auto file3 = make_shared<FileInfo>();
  file3->fileid = MAX_SHAREDFILE+1;
  file3->filepath = g_strdup("world");
  file3->packetn = 123;
  file3->filenum = 456;
  thread->AddPrivateFile(file3);

  auto file4 = thread->GetPrivateFileByPacketN(123, 456);
  EXPECT_STREQ(file4->filepath, file3->filepath);
}
