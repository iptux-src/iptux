#include "gtest/gtest.h"

#include "iptux/UiHelper.h"

using namespace std;
using namespace iptux;

TEST(UiHelper, utf8MakeValid) {
  ASSERT_EQ(utf8MakeValid(""), "");
  ASSERT_EQ(utf8MakeValid("中文"), "中文");
  ASSERT_EQ(utf8MakeValid("\xe4\xb8\xad\xe6\x96\x87"), "中文");
  ASSERT_EQ(utf8MakeValid("\xe4\xb8\xe6\x96\x87"), "��文");
  ASSERT_EQ(utf8MakeValid("\xe4\xb8\xad\xe6\x96"), "中��");
  ASSERT_EQ(utf8MakeValid("\xe4\xe6\x96\x87"), "�文");
  ASSERT_EQ(utf8MakeValid("\xe4\xb8\xad\xe6"), "中�");
}

TEST(UiHelper, markupEscapeText) {
  ASSERT_EQ(markupEscapeText(""), "");
  ASSERT_EQ(markupEscapeText("中文"), "中文");
  ASSERT_EQ(markupEscapeText("<span>"), "&lt;span&gt;");
  ASSERT_EQ(markupEscapeText("\"hello\""), "&quot;hello&quot;");
}
