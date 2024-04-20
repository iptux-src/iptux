#include "gtest/gtest.h"

#include "iptux/UiHelper.h"
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace iptux;

TEST(UiHelper, markupEscapeText) {
  ASSERT_EQ(markupEscapeText(""), "");
  ASSERT_EQ(markupEscapeText("中文"), "中文");
  ASSERT_EQ(markupEscapeText("<span>"), "&lt;span&gt;");
  ASSERT_EQ(markupEscapeText("\"hello\""), "&quot;hello&quot;");
}

TEST(UiHelper, TimeToStr) {
  setenv("TZ", "America/Vancouver", 1);
  tzset();
  ASSERT_EQ(TimeToStr_(1713583969, 1713583969), "20:32");
  ASSERT_EQ(TimeToStr_(1713583969 - 86400, 1713583969), "2024-04-18");
  ASSERT_EQ(TimeToStr_(1713583969 + 86400, 1713583969), "2024-04-20");
  setenv("TZ", "GMT", 1);
  tzset();
  ASSERT_EQ(TimeToStr_(1713583969, 1713583969), "03:32");
  ASSERT_EQ(TimeToStr_(1713583969 / 86400 * 86400, 1713583969), "00:00");
  ASSERT_EQ(TimeToStr_((1713583969 / 86400 + 1) * 86400 - 1, 1713583969),
            "23:59");
  ASSERT_EQ(TimeToStr_(1713583969 / 86400 * 86400 - 1, 1713583969),
            "2024-04-19");
  ASSERT_EQ(TimeToStr_((1713583969 / 86400 + 1) * 86400, 1713583969),
            "2024-04-21");
}

TEST(UiHelper, StrFirstNonEmptyLine) {
  ASSERT_EQ(StrFirstNonEmptyLine(""), "");
  ASSERT_EQ(StrFirstNonEmptyLine("a"), "a");
  ASSERT_EQ(StrFirstNonEmptyLine(" a"), "a");
  ASSERT_EQ(StrFirstNonEmptyLine(" a\n"), "a");
  ASSERT_EQ(StrFirstNonEmptyLine(" a \n"), "a ");
  ASSERT_EQ(StrFirstNonEmptyLine(" a \n b"), "a ");
  ASSERT_EQ(StrFirstNonEmptyLine("\n b"), "b");
  ASSERT_EQ(StrFirstNonEmptyLine("\n b"), "b");
  ASSERT_EQ(StrFirstNonEmptyLine(" \n b\n"), "b");
}
