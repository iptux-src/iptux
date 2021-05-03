#include "gtest/gtest.h"

#include "iptux-core/Models.h"
#include "iptux-utils/TestHelper.h"
#include "iptux-utils/utils.h"

using namespace std;
using namespace iptux;

TEST(PalInfo, GetKey) {
  PalInfo info;
  ASSERT_EQ(info.GetKey().ToString(), "0.0.0.0:2425");
}

TEST(PalKey, CopyConstructor) {
  PalKey key1(inAddrFromString("1.2.3.4"), 1234);
  PalKey key2 = key1;
  ASSERT_EQ(key1.ToString(), "1.2.3.4:1234");
  ASSERT_EQ(key2.ToString(), "1.2.3.4:1234");
}

TEST(NetSegment, ContainIP) {
  NetSegment netSegment("1.2.3.4", "1.2.4.5", "");

  vector<string> ips = {"1.2.3.4", "1.2.4.5", "1.2.3.255",
                        "1.2.4.0", "1.2.3.5", "1.2.4.4"};

  for (const string& ip : ips) {
    in_addr ip1;
    ASSERT_EQ(inet_pton(AF_INET, ip.c_str(), &ip1.s_addr), 1) << ip;
    ASSERT_TRUE(netSegment.ContainIP(ip1));
  }

  vector<string> ips2 = {
      "1.2.3.3",
      "1.2.4.6",
      "0.0.0.0",
      "100.100.100.100",
  };
  for (const string& ip : ips2) {
    in_addr ip1;
    ASSERT_EQ(inet_pton(AF_INET, ip.c_str(), &ip1), 1) << ip;
    ASSERT_FALSE(netSegment.ContainIP(ip1));
  }
}

TEST(ChipData, ToString) {
  EXPECT_EQ(ChipData("").ToString(), "ChipData(MessageContentType::STRING, )");
}

TEST(ChipData, getSummary) {
  EXPECT_EQ(ChipData("").getSummary(), "");
  EXPECT_EQ(ChipData("foobar").getSummary(), "foobar");
  EXPECT_EQ(ChipData(MessageContentType::PICTURE, "foobar").getSummary(),
            "Received an image");
}

TEST(FileAttr, Convert) {
  EXPECT_EQ(FileAttr(0), FileAttr::UNKNOWN);
  EXPECT_EQ(FileAttr(1), FileAttr::REGULAR);
  EXPECT_EQ(FileAttr(2), FileAttr::DIRECTORY);
  EXPECT_EQ(FileAttr(3), FileAttr(3));
  EXPECT_EQ(FileAttr(-1), FileAttr(-1));
}

TEST(MsgPara, getSummary) {
  PPalInfo pal = make_shared<PalInfo>();
  MsgPara msg(pal);
  EXPECT_EQ(msg.getSummary(), "Empty Message");
  msg.dtlist.push_back(ChipData("foobar"));
  EXPECT_EQ(msg.getSummary(), "foobar");
  msg.dtlist.push_back(ChipData("foobar2"));
  EXPECT_EQ(msg.getSummary(), "foobar");
  msg.dtlist.clear();
  msg.dtlist.push_back(ChipData(MessageContentType::PICTURE, "foobar"));
  EXPECT_EQ(msg.getSummary(), "Received an image");
}

TEST(FileInfo, isExist) {
  auto path1 = testDataPath("hexdumptest.dat");
  FileInfo f;
  f.filepath = g_strdup(path1.c_str());
  ASSERT_TRUE(f.isExist());

  auto path2 = testDataPath("hexdumptest-notexist.dat");
  FileInfo f2;
  f2.filepath = g_strdup(path2.c_str());
  ASSERT_FALSE(f2.isExist());
}

TEST(FileInfo, ensureFilesizeFilled) {
  auto path1 = testDataPath("hexdumptest.dat");
  FileInfo f;
  f.filepath = g_strdup(path1.c_str());
  f.ensureFilesizeFilled();
  ASSERT_EQ(f.filesize, 256);
}
