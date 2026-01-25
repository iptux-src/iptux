#include "gtest/gtest.h"

#include "iptux-core/Models.h"
#include "iptux-utils/TestHelper.h"
#include "iptux-utils/utils.h"

using namespace std;
using namespace iptux;

TEST(PalInfo, GetKey) {
  PalInfo info("127.0.0.1", 2425);
  ASSERT_EQ(info.GetKey().ToString(), "127.0.0.1:2425");
}

TEST(PalKey, CopyConstructor) {
  PalKey key1(inAddrFromString("1.2.3.4"), 1234);
  PalKey key2 = key1;
  ASSERT_EQ(key1.ToString(), "1.2.3.4:1234");
  ASSERT_EQ(key2.ToString(), "1.2.3.4:1234");
}

TEST(PalKey, GetIpv4String) {
  PalKey key1(inAddrFromString("1.2.3.4"), 1234);
  ASSERT_EQ(key1.GetIpv4String(), "1.2.3.4");
}

TEST(PalKey, GSocketAddressConstructor) {
  // Test creating PalKey from GSocketAddress
  GInetAddress* inet_addr = g_inet_address_new_from_string("192.168.1.100");
  ASSERT_NE(inet_addr, nullptr);

  GSocketAddress* socket_addr = g_inet_socket_address_new(inet_addr, 8080);
  ASSERT_NE(socket_addr, nullptr);

  PalKey key(socket_addr);

  // Test that the values are correctly extracted
  ASSERT_EQ(key.GetIpv4String(), "192.168.1.100");
  ASSERT_EQ(key.GetPort(), 8080);
  ASSERT_EQ(key.ToString(), "192.168.1.100:8080");

  // Test GetSocketAddress method
  GSocketAddress* retrieved_addr = key.GetSocketAddress();
  ASSERT_NE(retrieved_addr, nullptr);

  // Verify the retrieved address matches
  GInetSocketAddress* inet_socket_addr = G_INET_SOCKET_ADDRESS(retrieved_addr);
  ASSERT_EQ(g_inet_socket_address_get_port(inet_socket_addr), 8080);

  GInetAddress* retrieved_inet =
      g_inet_socket_address_get_address(inet_socket_addr);
  gchar* ip_str = g_inet_address_to_string(retrieved_inet);
  ASSERT_STREQ(ip_str, "192.168.1.100");
  g_free(ip_str);

  // Test equality with another PalKey created from in_addr
  PalKey key2(inAddrFromString("192.168.1.100"), 8080);
  ASSERT_EQ(key, key2);

  // Cleanup
  g_object_unref(retrieved_addr);
  g_object_unref(socket_addr);
  g_object_unref(inet_addr);
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
  auto pal = make_shared<PalInfo>("127.0.0.1", 2425);
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
