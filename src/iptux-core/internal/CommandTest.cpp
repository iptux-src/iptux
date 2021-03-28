#include "gtest/gtest.h"

#include "Command.h"

#include "iptux-core/internal/ipmsg.h"

using namespace iptux;
using namespace std;

TEST(Command, encodFileInfo) {
  FileInfo fileInfo;
  fileInfo.filepath = g_strdup("/etc/bashrc");
  fileInfo.fileattr = FileAttr::REGULAR;
  auto a = Command::encodeFileInfo(fileInfo);
  ASSERT_EQ(a, "0:bashrc:ffffffffffffffff:0:1:\a:");

  vector<FileInfo> fileInfos = Command::decodeFileInfos(a);
  ASSERT_EQ(fileInfos.size(), 1);
  ASSERT_EQ(fileInfos[0], fileInfo);

  fileInfos = Command::decodeFileInfos(a + a);
  ASSERT_EQ(fileInfos.size(), 2);
  ASSERT_EQ(fileInfos[0], fileInfo);
  ASSERT_EQ(fileInfos[1], fileInfo);

  string b = "0:bashrc:ffffffffffffffff:0:0:\a:";
  fileInfos = Command::decodeFileInfos(b);
  ASSERT_EQ(fileInfos.size(), 0);
}
