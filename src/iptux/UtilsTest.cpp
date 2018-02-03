#include "gtest/gtest.h"

#include "iptux/utils.h"

using namespace iptux;

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
