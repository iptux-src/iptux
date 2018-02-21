#include "gtest/gtest.h"

#include "iptux/ProgramData.h"
#include "iptux/TestHelper.h"

using namespace iptux;

TEST(ProgramData, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  ProgramData* data = new ProgramData(*config);
  ASSERT_NE(data->lcursor, nullptr);
  ASSERT_EQ(gdk_cursor_get_cursor_type(data->lcursor), GDK_HAND2);
  delete data;
}

