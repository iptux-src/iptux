#include "gtest/gtest.h"

#include "iptux-utils/TestHelper.h"
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
  setenv("TZ", "PST8PDT,M3.2.0/2,M11.1.0/2", 1);
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

TEST(UiHelper, igtk_image_new_with_size) {
  auto image =
      igtk_image_new_with_size(testDataPath("iptux.png").c_str(), 100, 100);
  ASSERT_NE(image, nullptr);
  auto pixbuf = gtk_image_get_pixbuf(image);
  ASSERT_EQ(gdk_pixbuf_get_width(pixbuf), 48);
  ASSERT_EQ(gdk_pixbuf_get_height(pixbuf), 48);
  g_object_unref(pixbuf);
  g_object_unref(image);

  image = igtk_image_new_with_size(testDataPath("iptux.png").c_str(), 20, 30);
  ASSERT_NE(image, nullptr);
  pixbuf = gtk_image_get_pixbuf(image);
  ASSERT_EQ(gdk_pixbuf_get_width(pixbuf), 20);
  ASSERT_EQ(gdk_pixbuf_get_height(pixbuf), 20);
  g_object_unref(pixbuf);
  g_object_unref(image);
}
