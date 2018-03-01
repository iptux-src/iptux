#include <string>

#include <glib.h>

#include "gtest/gtest.h"

#include "iptux/IptuxConfig.h"

#include "iptux/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(IptuxConfig, SetStringList) {
  auto config = newTestIptuxConfig();
  config->SetStringList("key", vector<string>());
  EXPECT_EQ(config->GetStringList("key").size(), 0);
  config->SetStringList("key", vector<string>{"hello", "world!"});
  EXPECT_EQ(config->GetStringList("key").size(), 2);
  config->SetStringList("key", vector<string>{"hello"});
  EXPECT_EQ(config->GetStringList("key").size(), 1);

  const char* boolKey = "boolKey";
  ASSERT_FALSE(config->GetBool(boolKey));
  ASSERT_FALSE(config->GetBool(boolKey, false));
  ASSERT_TRUE(config->GetBool(boolKey, true));
  config->SetBool(boolKey, false);
  ASSERT_FALSE(config->GetBool(boolKey));
  ASSERT_FALSE(config->GetBool(boolKey, false));
  ASSERT_FALSE(config->GetBool(boolKey, true));
  config->SetBool(boolKey, true);
  ASSERT_TRUE(config->GetBool(boolKey));
  ASSERT_TRUE(config->GetBool(boolKey, false));
  ASSERT_TRUE(config->GetBool(boolKey, true));
}
