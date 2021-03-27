#include "gtest/gtest.h"

#include "iptux/UiHelper.h"

using namespace std;
using namespace iptux;

TEST(UiHelper, markupEscapeText) {
  ASSERT_EQ(markupEscapeText(""), "");
  ASSERT_EQ(markupEscapeText("中文"), "中文");
  ASSERT_EQ(markupEscapeText("<span>"), "&lt;span&gt;");
  ASSERT_EQ(markupEscapeText("\"hello\""), "&quot;hello&quot;");
}
