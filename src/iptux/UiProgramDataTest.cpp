#include "gtest/gtest.h"

#include "iptux/UiProgramData.h"
#include "iptux-core/TestHelper.h"

using namespace iptux;

TEST(UiProgramData, Constructor) {
  gtk_init(nullptr, nullptr);
  auto config = newTestIptuxConfig();
  UiProgramData* data = new UiProgramData(config);
  ASSERT_NE(data->table, nullptr);
  delete data;
}
