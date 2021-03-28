#include <cstdio>
#include <iostream>
#include <string>

#include <unistd.h>

#include <glog/logging.h>
#include <iptux-core/CoreThread.h>
#include <iptux-core/Exception.h>

using namespace std;
using namespace iptux;

void usage(const char* progname) {
  printf("Usage:\n");
  printf("  %s [IP]  -- run robot on IP(default: 0.0.0.0)\n", progname);
  printf("  %s -h    -- print help\n", progname);
}

void processNewMessageEvent(shared_ptr<CoreThread> ct,
                            const NewMessageEvent* event) {
  auto para = event->getMsgPara();
  LOG(INFO) << "New Message Event: " << endl;
  LOG(INFO) << "  From: " << para.pal->GetKey().ToString() << endl;
  for (auto& chip : para.dtlist) {
    LOG(INFO) << "  Message: " << chip.ToString() << endl;
    ostringstream oss;
    oss << "your message has " << chip.data.size() << " bytes.";
    ct->SendMessage(para.pal, oss.str());
  }
}

void processEvent(shared_ptr<CoreThread> ct, shared_ptr<const Event> event) {
  cout << "Event: " << int(event->getType()) << endl;
  if (event->getType() == EventType::NEW_MESSAGE) {
    processNewMessageEvent(ct,
                           dynamic_cast<const NewMessageEvent*>(event.get()));
  }
}

int runBot(const string& bindIp) {
  auto config = IptuxConfig::newFromString("{}");
  config->SetString("bind_ip", bindIp);
  auto progdt = make_shared<ProgramData>(config);
  auto thread = make_shared<CoreThread>(progdt);
  thread->start();
  thread->registerCallback(
      [=](shared_ptr<const Event> event) { processEvent(thread, event); });
  while (true) {
    sleep(10);
  }
}

int main(int argc, char* argv[]) {
  if (argc == 2 && string("-h") == argv[1]) {
    usage(argv[0]);
    return 0;
  }

  if (argc > 2) {
    usage(argv[0]);
    return 1;
  }

  string bindIp("0.0.0.0");
  if (argc == 2) {
    bindIp = argv[1];
  }

  return runBot(bindIp);
}
