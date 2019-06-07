#include "gtest/gtest.h"

#include "iptux-core/utils.h"
#include "iptux/Exception.h"
#include "iptux/TestHelper.h"

using namespace iptux;
using namespace std;

TEST(Utils, FLAG_SET) {
  uint8_t a = 1;
  FLAG_SET(a, 0, false);
  EXPECT_EQ(a, 0);

  a = 1;
  FLAG_SET(a, 1, true);
  EXPECT_EQ(a, 3);
}

TEST(Utils, numeric_to_size) {
  EXPECT_STREQ(numeric_to_size(0), "0B");
  EXPECT_STREQ(numeric_to_size(1), "1B");
  EXPECT_STREQ(numeric_to_size(1000), "1000B");
  EXPECT_STREQ(numeric_to_size(1<<10), "1.0KiB");
  EXPECT_STREQ(numeric_to_size(1000000), "976.6KiB");
  EXPECT_STREQ(numeric_to_size(1<<20), "1.0MiB");
  EXPECT_STREQ(numeric_to_size(1000000000), "953.7MiB");
  EXPECT_STREQ(numeric_to_size(1<<30), "1.0GiB");
  EXPECT_STREQ(numeric_to_size(1000000000000L), "931.3GiB");
  EXPECT_STREQ(numeric_to_size(1L<<40), "1.0TiB");
  EXPECT_STREQ(numeric_to_size(-1), "-1B");
  EXPECT_STREQ(numeric_to_size(-1024), "-1024B");
}

TEST(Utils, stringToInAddr) {
  EXPECT_EQ(stringToInAddr("127.0.0.1"), 0x100007fu);
  EXPECT_EQ(stringToInAddr("1.2.3.4"), 0x4030201u);
  EXPECT_EQ(stringToInAddr("1.2.3.255"), 0xff030201u);
  EXPECT_EQ(stringToInAddr("255.2.3.4"), 0x40302ffu);

  vector<string> cases = {
      "",
      "123",
      "123.234",
      "123.234.2",
      "123.235.0.256",
      "1.2.3.4.5",
      "hello world",
  };

  for(const string& c: cases) {
    try {
      stringToInAddr(c);
      EXPECT_TRUE(false) << c;
    } catch(Exception& e) {
      ASSERT_EQ(e.getErrorCode(), ErrorCode::INVALID_IP_ADDRESS);
    }
  }
}

TEST(Utils, stringFormat) {
  EXPECT_EQ(stringFormat("hello"), "hello");
  EXPECT_EQ(stringFormat("hello %s", "world"), "hello world");
  EXPECT_EQ(stringFormat("hello %d", 3), "hello 3");
  // following will SIGSEGV
  // EXPECT_EQ(stringFormat("hello%s"), "hello");
  // EXPECT_EQ(stringFormat("hello %d", "world"), "hello 3");
}

TEST(Utils, stringDump) {
  EXPECT_EQ(stringDump(""), "");
  EXPECT_EQ(stringDump("\n"), "00000000  0a                                                |.|\n00000001\n");
  EXPECT_EQ(stringDump(readTestData("hexdumptest.dat")), readTestData("hexdumptest.out.dat"));
}

string construct00ToFF() {
  ostringstream oss;
  for(int i = 0; i < 256; ++i) {
    oss << char(i);
  }
  return oss.str();
}

TEST(Utils, stringDumpAsCString) {
  EXPECT_EQ(stringDumpAsCString(""), "\"\"");
  EXPECT_EQ(stringDumpAsCString("\""), "\"\\\"\"");
  auto out = readTestData("hexcstringtest.out.dat");
  EXPECT_EQ(stringDumpAsCString(construct00ToFF()), out.substr(0, out.length()-1));
  auto input = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\t\n\x0b\x0c\r\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7f\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";
  EXPECT_EQ(stringDumpAsCString(string(input, 256)),
    out.substr(0, out.length()-1));
}
