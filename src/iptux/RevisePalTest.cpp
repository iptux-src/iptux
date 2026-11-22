#include "gtest/gtest.h"

#include "iptux-core/TestHelper.h"
#include "iptux/Application.h"
#include "iptux/RevisePal.h"
#include "iptux/TestHelper.h"

using namespace std;
using namespace iptux;

namespace iptux {

class RevisePalTestPeer {
 public:
  static GtkWidget* CreateAllArea(RevisePal& pal) {
    return pal.CreateAllArea();
  }
  static void SetAllValue(RevisePal& pal) { pal.SetAllValue(); }
  static GtkWidget* GetWidget(RevisePal& pal, const char* key) {
    return GTK_WIDGET(g_datalist_get_data(&pal.widset, key));
  }
  static GtkWidget* FindButtonByLabel(GtkWidget* container, const char* label) {
    GList* children = gtk_container_get_children(GTK_CONTAINER(container));
    for (GList* child = children; child; child = child->next) {
      GtkWidget* widget = GTK_WIDGET(child->data);
      if (GTK_IS_BUTTON(widget) &&
          g_strcmp0(gtk_button_get_label(GTK_BUTTON(widget)), label) == 0) {
        g_list_free(children);
        return widget;
      }
    }
    g_list_free(children);
    return nullptr;
  }
};

}  // namespace iptux

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

  auto area = RevisePalTestPeer::CreateAllArea(pal);
  EXPECT_TRUE(GTK_IS_GRID(area));

  auto widget = RevisePalTestPeer::GetWidget(pal, "protocol-combo-widget");
  ASSERT_TRUE(GTK_IS_COMBO_BOX(widget));
  auto model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
  EXPECT_EQ(gtk_tree_model_iter_n_children(model, nullptr), 3);

  gtk_widget_destroy(area);
  DestroyApplication(app);
}

TEST(RevisePal, IconButtonKeepsIconComboBinding) {
  Application* app = CreateApplication();

  PalInfo palInfo("127.0.0.1", 2425);
  RevisePal pal(app, nullptr, &palInfo);

  auto area = RevisePalTestPeer::CreateAllArea(pal);
  auto iconCombo = RevisePalTestPeer::GetWidget(pal, "icon-combo-widget");
  auto button = RevisePalTestPeer::FindButtonByLabel(area, "...");

  ASSERT_TRUE(GTK_IS_BUTTON(button));
  EXPECT_EQ(g_object_get_data(G_OBJECT(button), "icon-combo-widget"),
            iconCombo);

  gtk_widget_destroy(area);
  DestroyApplication(app);
}

TEST(RevisePal, ProtocolSelectionReflectsPalProtocol) {
  Application* app = CreateApplication();

  PalInfo palInfo("127.0.0.1", 2425);
  palInfo.setProtocol(PalProtocol::IPTUX);
  RevisePal pal(app, nullptr, &palInfo);

  auto area = RevisePalTestPeer::CreateAllArea(pal);
  RevisePalTestPeer::SetAllValue(pal);

  auto widget = RevisePalTestPeer::GetWidget(pal, "protocol-combo-widget");
  ASSERT_EQ(gtk_combo_box_get_active(GTK_COMBO_BOX(widget)), 2);

  palInfo.setProtocol(PalProtocol::AUTO);
  RevisePalTestPeer::SetAllValue(pal);
  EXPECT_EQ(gtk_combo_box_get_active(GTK_COMBO_BOX(widget)), 0);

  gtk_widget_destroy(area);
  DestroyApplication(app);
}
