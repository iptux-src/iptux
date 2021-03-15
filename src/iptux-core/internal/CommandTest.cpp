#include "gtest/gtest.h"

#include "Command.h"

#include "iptux-core/internal/ipmsg.h"

using namespace iptux;
using namespace std;

TEST(Command, encodFileInfo) {
  FileInfo fileInfo;
  fileInfo.filepath = g_strdup("/etc/bashrc");
  auto a = Command::encodeFileInfo(fileInfo);
  ASSERT_EQ(a, "0:bashrc:ffffffffffffffff:0:0:\a:");
}
