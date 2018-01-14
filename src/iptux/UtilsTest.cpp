#include "gtest/gtest.h"

#include "iptux/utils.h"

using namespace iptux;

TEST(FLAG_SET, Utils)
{
  uint8_t a = 1;
  FLAG_SET(a, 0, false);
  EXPECT_EQ(a, 0);

  a = 1;
  FLAG_SET(a, 1, true);
  EXPECT_EQ(a, 3);
}
