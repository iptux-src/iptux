#include "config.h"
#include "TestConfig.h"
#include "TestHelper.h"

#include <fstream>
#include <iostream>
#include <thread>

#include <glib.h>

#include "iptux/utils.h"
#include "iptux-core/support.h"

using namespace std;

namespace iptux {

shared_ptr<IptuxConfig> newTestIptuxConfig() {
  auto res = IptuxConfig::newFromString("{}");
  res->SetBool("debug_dont_broadcast", true);
  return res;
}

shared_ptr<IptuxConfig> newTestIptuxConfigWithFile() {
  char* fname = g_strdup_printf("/tmp/iptux%d.json", g_random_int());
  auto res = make_shared<IptuxConfig>(fname);
  res->SetBool("debug_dont_broadcast", true);
  g_free(fname);
  return res;
}

string readTestData(const string& fname) {
  ifstream ifs(testDataPath(fname));
  return string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}

string testDataPath(const string& fname) {
  return stringFormat("%s/src/iptux/testdata/%s", PROJECT_ROOT_PATH, fname.c_str());
}

std::shared_ptr<CoreThread> newCoreThreadOnIp(const std::string& ip) {
  auto config = newTestIptuxConfig();
  config->SetString("bind_ip", ip);
  return make_shared<CoreThread>(make_shared<ProgramData>(config));
}

std::tuple<PCoreThread, PCoreThread>
initAndConnnectThreadsFromConfig(PIptuxConfig c1, PIptuxConfig c2) {
  c1->SetBool("debug_dont_broadcast", true);
  c2->SetBool("debug_dont_broadcast", true);
  auto thread1 = make_shared<CoreThread>(make_shared<ProgramData>(c1));
  auto thread2 = make_shared<CoreThread>(make_shared<ProgramData>(c2));
  try {
    thread2->start();
  } catch(BindFailedException& e) {
    cerr
      << "bind to "<< c2->GetString("bind_ip") << " failed.\n"
      << "if you are using mac, please run `sudo ifconfig lo0 alias " << c2->GetString("bind_ip")
      << " up` first.\n";
    throw;
  }
  thread1->start();
  thread1->SendDetectPacket(c2->GetString("bind_ip"));
  while(thread2->GetOnlineCount() != 1) {
    this_thread::sleep_for(10ms);
  }

  return make_tuple(thread1, thread2);
}


}

