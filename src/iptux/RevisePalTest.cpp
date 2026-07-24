#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux/Application.h"
#define private public
#include "iptux/RevisePal.h"
#undef private
#include "iptux/TestHelper.h"

using namespace std;
using namespace iptux;

TEST(RevisePal, Constructor) {
  Application* app = CreateApplication();

  PalInfo palInfo("127.0.0.1", 2425);

  RevisePal pal(app, nullptr, &palInfo);
  DestroyApplication(app);
}

TEST(RevisePal, ReviseEntryDo) {
  Application* app = CreateApplication();

  PalInfo palInfo("127.0.0.1", 2425);

  RevisePal::ReviseEntryDo(app, nullptr, &palInfo, false);
  DestroyApplication(app);
}

TEST(RevisePal, CreateAllAreaUsesGridAndProtocolCombo) {
  Application* app = CreateApplication();

  PalInfo palInfo("127.0.0.1", 2425);
  RevisePal pal(app, nullptr, &palInfo);

  auto area = pal.CreateAllArea();
  EXPECT_TRUE(GTK_IS_GRID(area));

  auto widget =
      GTK_WIDGET(g_datalist_get_data(&pal.widset, "protocol-combo-widget"));
  ASSERT_TRUE(GTK_IS_COMBO_BOX(widget));
  auto model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
  EXPECT_EQ(gtk_tree_model_iter_n_children(model, nullptr), 3);

  gtk_widget_destroy(area);
  DestroyApplication(app);
}

TEST(RevisePal, ProtocolSelectionReflectsPalProtocol) {
  Application* app = CreateApplication();

  PalInfo palInfo("127.0.0.1", 2425);
  palInfo.setProtocol(PalProtocol::IPTUX);
  RevisePal pal(app, nullptr, &palInfo);

  auto area = pal.CreateAllArea();
  pal.SetAllValue();

  auto widget =
      GTK_WIDGET(g_datalist_get_data(&pal.widset, "protocol-combo-widget"));
  ASSERT_EQ(gtk_combo_box_get_active(GTK_COMBO_BOX(widget)), 2);

  palInfo.setProtocol(PalProtocol::AUTO);
  pal.SetAllValue();
  EXPECT_EQ(gtk_combo_box_get_active(GTK_COMBO_BOX(widget)), 0);

  gtk_widget_destroy(area);
  DestroyApplication(app);
}
