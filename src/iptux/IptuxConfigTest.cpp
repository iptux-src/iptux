#include <string>

#include <glib.h>

#include "gtest/gtest.h"

#include "iptux/IptuxConfig.h"

using namespace std;
using namespace iptux;

TEST(IptuxConfig, SetStringList) {
  char templateFname[] = "/tmp/iptuxXXXXXX.json";
  char* tmpfile = mktemp(templateFname);
  string fname(tmpfile);
  IptuxConfig config(fname);
  config.SetStringList("key", vector<string>());
  EXPECT_EQ(config.GetStringList("key").size(), 0);
  config.SetStringList("key", vector<string>{"hello", "world!"});
  EXPECT_EQ(config.GetStringList("key").size(), 2);
  config.SetStringList("key", vector<string>{"hello"});
  EXPECT_EQ(config.GetStringList("key").size(), 1);
}
