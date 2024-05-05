#include <string>

#include <glib.h>

#include "gtest/gtest.h"

#include "iptux-core/IptuxConfig.h"

#include "iptux-core/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(IptuxConfig, SetStringList) {
  auto config = newTestIptuxConfig();
  config->SetStringList("key", vector<string>());
  EXPECT_EQ(int(config->GetStringList("key").size()), 0);
  config->SetStringList("key", vector<string>{"hello", "world!"});
  EXPECT_EQ(int(config->GetStringList("key").size()), 2);
  config->SetStringList("key", vector<string>{"hello"});
  EXPECT_EQ(int(config->GetStringList("key").size()), 1);
  config->SetString("key", "hello");
  EXPECT_EQ(int(config->GetStringList("key").size()), 0);
  config->SetInt("key", 1);
  EXPECT_EQ(int(config->GetStringList("key").size()), 0);
}

TEST(IptuxConfig, GetBool) {
  auto config = newTestIptuxConfig();
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
  config->SetString(boolKey, "a");
  ASSERT_FALSE(config->GetBool(boolKey));
}

TEST(IptuxConfig, GetInt) {
  auto config = newTestIptuxConfig();
  const char* intKey = "intKey";
  config->SetString(intKey, "a");
  ASSERT_EQ(config->GetInt(intKey, 1234), 1234);
  config->SetString(intKey, "1");
  ASSERT_EQ(config->GetInt(intKey, 1234), 1234);
}

TEST(IptuxConfig, GetString) {
  auto config = newTestIptuxConfig();
  const char* stringKey = "stringKey";
  ASSERT_EQ(config->GetString(stringKey), "");
  ASSERT_EQ(config->GetString(stringKey, "world"), "world");
  config->SetString(stringKey, "hello");
  ASSERT_EQ(config->GetString(stringKey), "hello");
  ASSERT_EQ(config->GetString(stringKey, "world"), "hello");
  config->SetInt(stringKey, 1);
  ASSERT_EQ(config->GetString(stringKey), "");
  ASSERT_EQ(config->GetString(stringKey, "world"), "world");
  config->SetBool(stringKey, false);
  ASSERT_EQ(config->GetString(stringKey), "");
  ASSERT_EQ(config->GetString(stringKey, "world"), "world");
  config->SetStringList(stringKey, vector<string>{"hello", "world"});
  ASSERT_EQ(config->GetString(stringKey), "");
  ASSERT_EQ(config->GetString(stringKey, "world"), "world");
}
