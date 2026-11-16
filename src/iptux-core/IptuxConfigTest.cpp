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

TEST(IptuxConfig, VolatileKey) {
  auto config = newTestIptuxConfigWithFile();
  config->SetString("normalKey", "hello");
  config->MarkKeyVolatile("volatileKey");
  config->SetString("volatileKey", "world");
  config->Save();

  auto config2 = make_shared<IptuxConfig>(config->getFileName());
  EXPECT_EQ(config2->GetString("normalKey"), "hello");
  EXPECT_EQ(config2->GetString("volatileKey"), "");
  EXPECT_TRUE(config->GetString("volatileKey") == "world");
}

TEST(IptuxConfig, VolatileKeyRestoreLoadedValue) {
  auto config = newTestIptuxConfigWithFile();
  config->SetString("key", "loaded");
  config->Save();

  auto config2 = make_shared<IptuxConfig>(config->getFileName());
  EXPECT_EQ(config2->GetString("key"), "loaded");
  config2->MarkKeyVolatile("key");
  config2->SetString("key", "override");
  EXPECT_EQ(config2->GetString("key"), "override");
  config2->Save();

  auto config3 = make_shared<IptuxConfig>(config->getFileName());
  EXPECT_EQ(config3->GetString("key"), "loaded");
  EXPECT_EQ(config2->GetString("key"), "override");
}

TEST(IptuxConfig, VolatileKeyStaysVolatile) {
  auto config = newTestIptuxConfigWithFile();
  config->MarkKeyVolatile("key");
  config->SetString("key", "override");
  // even a later change (e.g. from the preferences dialog) is not
  // persisted, the key was never loaded from the file
  config->SetString("key", "user-changed");
  config->Save();

  auto config2 = make_shared<IptuxConfig>(config->getFileName());
  EXPECT_EQ(config2->GetString("key"), "");
  EXPECT_EQ(config->GetString("key"), "user-changed");
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
