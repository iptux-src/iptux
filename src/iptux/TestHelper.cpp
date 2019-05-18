#include "config.h"
#include "TestConfig.h"
#include "TestHelper.h"

#include "iptux/utils.h"

#include <fstream>

#include <glib.h>

using namespace std;

namespace iptux {

shared_ptr<IptuxConfig> newTestIptuxConfig() {
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

}

