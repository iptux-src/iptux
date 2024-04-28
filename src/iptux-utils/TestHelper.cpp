#include "config.h"
#include "TestHelper.h"
#include "TestConfig.h"

#include <fstream>
#include <iostream>
#include <thread>

#include <glib.h>

#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

string readTestData(const string& fname) {
  ifstream ifs(testDataPath(fname));
  if (!ifs) {
    throw runtime_error(stringFormat("Cannot open file %s", fname.c_str()));
  }
  return string(std::istreambuf_iterator<char>(ifs),
                std::istreambuf_iterator<char>());
}

string testDataPath(const string& fname) {
  return stringFormat("%s/iptux/testdata/%s", CURRENT_SOURCE_PATH,
                      fname.c_str());
}

}  // namespace iptux
