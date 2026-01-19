// SPDX-License-Identifier: GPL-2.0-or-later

#include "Encrypt.h"
#include "gtest/gtest.h"

struct GenRsaPrivPemTest {
  int bits;
  bool success;
  bool fail;
};

TEST(Encrypt, GenRsaPrivPem) {
  struct GenRsaPrivPemTest tests[] = {
      {1024, .success = 1}, {2048, .success = 1}, {4096, .success = 1},
      {512, .success = 1},     {0, .fail = 1},       {-1, .fail = 1},
      {1025, .success = 1},    {1032, .success = 1},
  };

  for (const auto& test : tests) {
    std::string priv_pem = iptux::Encrypt::genRsaPrivPem(test.bits);
    if (test.success) {
      ASSERT_FALSE(priv_pem.empty()) << "bits=" << test.bits;
    }

    if (test.fail) {
      ASSERT_TRUE(priv_pem.empty()) << "bits=" << test.bits;
    }
  }
}