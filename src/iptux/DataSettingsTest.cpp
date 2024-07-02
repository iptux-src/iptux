#include "TestConfig.h"
#include "UiHelper.h"
#include "gtest/gtest.h"

#include "iptux/Application.h"
#include "iptux/DataSettings.h"
#include "iptux/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(DataSettings, Constructor) {
  Application* app = CreateApplication();
  DataSettings ds(app, nullptr);
  ASSERT_TRUE(ds.Save());
  DestroyApplication(app);
}

#if CONFIG_DEBUG
TEST(DataSettings, Constructor2) {
  pop_disable();
  Application* app = CreateApplication();
  DataSettings ds(app, nullptr);
  GtkEntry* port_entry = GTK_ENTRY(ds.GetWidget("port-entry-widget"));
  gtk_entry_set_text(port_entry, "abc");
  ASSERT_FALSE(ds.Save());
}
#endif
