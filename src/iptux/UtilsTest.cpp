#include "gtest/gtest.h"

#include "iptux/utils.h"
#include "Exception.h"

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
  EXPECT_EQ(stringToInAddr("127.0.0.1"), 0x100007f);
  EXPECT_EQ(stringToInAddr("1.2.3.4"), 0x4030201);
  EXPECT_EQ(stringToInAddr("1.2.3.255"), 0xff030201);
  EXPECT_EQ(stringToInAddr("255.2.3.4"), 0x40302ff);

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
