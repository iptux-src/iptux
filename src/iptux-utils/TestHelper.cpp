#include "config.h"
#include "TestConfig.h"
#include "TestHelper.h"

#include <fstream>
#include <iostream>
#include <thread>

#include <glib.h>

#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

string readTestData(const string& fname) {
  ifstream ifs(testDataPath(fname));
  return string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}

string testDataPath(const string& fname) {
  return stringFormat("%s/src/iptux/testdata/%s", PROJECT_ROOT_PATH, fname.c_str());
}

}

