#include "config.h"
#include "TestHelper.h"
#include "TestConfig.h"

#include <fstream>
#include <iostream>
#include <thread>

#include <glib.h>

#include "iptux-core/Exception.h"
#include "iptux-core/internal/support.h"
#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

shared_ptr<IptuxConfig> newTestIptuxConfig() {
  auto res = IptuxConfig::newFromString("{}");
  res->SetBool("debug_dont_broadcast", true);
  res->SetInt("send_message_retry_in_us", 1000 * 10);
  return res;
}

shared_ptr<IptuxConfig> newTestIptuxConfigWithFile() {
  char* fname = g_strdup_printf("/tmp/iptux%d.json", g_random_int());
  auto res = make_shared<IptuxConfig>(fname);
  res->SetBool("debug_dont_broadcast", true);
  g_free(fname);
  return res;
}

std::shared_ptr<CoreThread> newCoreThread() {
  auto config = newTestIptuxConfig();
  return make_shared<CoreThread>(make_shared<ProgramData>(config));
}

std::shared_ptr<CoreThread> newCoreThreadOnIp(const std::string& ip) {
  auto config = newTestIptuxConfig();
  config->SetString("bind_ip", ip);
  return make_shared<CoreThread>(make_shared<ProgramData>(config));
}

std::tuple<PCoreThread, PCoreThread> initAndConnnectThreadsFromConfig(
    PIptuxConfig c1,
    PIptuxConfig c2) {
  c1->SetBool("debug_dont_broadcast", true);
  c2->SetBool("debug_dont_broadcast", true);
  auto thread1 = make_shared<CoreThread>(make_shared<ProgramData>(c1));
  auto thread2 = make_shared<CoreThread>(make_shared<ProgramData>(c2));
  try {
    thread2->start();
  } catch (Exception& e) {
    cerr << "bind to " << c2->GetString("bind_ip") << " failed.\n"
         << "if you are using mac, please run `sudo ifconfig lo0 alias "
         << c2->GetString("bind_ip") << " up` first.\n";
    throw;
  }
  thread1->start();
  while (thread2->GetOnlineCount() != 1) {
    thread1->SendDetectPacket(c2->GetString("bind_ip"));
    this_thread::sleep_for(10ms);
  }
  while (thread1->GetOnlineCount() != 1) {
    thread2->SendDetectPacket(c1->GetString("bind_ip"));
    this_thread::sleep_for(10ms);
  }
  return make_tuple(thread1, thread2);
}

}  // namespace iptux
