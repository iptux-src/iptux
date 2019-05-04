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
  auto res = shared_ptr<IptuxConfig>(new IptuxConfig(fname));
  g_free(fname);
  return res;
}

string readTestData(const string& fname) {
  auto fullPath = stringFormat("%s/src/iptux/testdata/%s", PROJECT_ROOT_PATH, fname.c_str());
  ifstream ifs(fullPath);
  return string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}

}

