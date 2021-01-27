#ifndef IPTUX_UTILS_TESTHELPER_H
#define IPTUX_UTILS_TESTHELPER_H

#include <string>

namespace iptux {
  std::string readTestData(const std::string& fname);
  std::string testDataPath(const std::string& fname);
}


#endif //IPTUX_TESTHELPER_H
