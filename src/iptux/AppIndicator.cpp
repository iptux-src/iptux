#include "config.h"
#include "AppIndicator.h"

#include <glib/gi18n.h>
#include <libayatana-appindicator/app-indicator.h>

#include "iptux/Application.h"
#include "iptux-utils/utils.h"

namespace iptux {

class IptuxAppIndicatorPrivate {
 public:
  IptuxAppIndicatorPrivate(Application* app) : app(app) {}
  ~IptuxAppIndicatorPrivate() {
    if (indicator) {
      g_object_unref(indicator);
    }
    if (menuBuilder) {
      g_object_unref(menuBuilder);
    }
  }
  Application* app;
  AppIndicator* indicator;
  GtkBuilder* menuBuilder;
};

IptuxAppIndicator::IptuxAppIndicator(Application* app) {
  this->priv = std::make_shared<IptuxAppIndicatorPrivate>(app);
  priv->indicator = app_indicator_new("io.github.iptux_src.iptux", "iptux-icon",
                                      APP_INDICATOR_CATEGORY_COMMUNICATIONS);
  app_indicator_set_status(priv->indicator, APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_attention_icon(priv->indicator, "iptux-attention");

  app_indicator_set_title(priv->indicator, _("Iptux"));

  priv->menuBuilder =
      gtk_builder_new_from_resource(IPTUX_RESOURCE "gtk/AppIndicator.ui");

  GtkMenu* menu = GTK_MENU(gtk_menu_new_from_model(G_MENU_MODEL(
      gtk_builder_get_object(priv->menuBuilder, "app-indicator-menu"))));
  gtk_widget_insert_action_group(GTK_WIDGET(menu), "app",
                                 G_ACTION_GROUP(app->getApp()));
  app_indicator_set_menu(priv->indicator, menu);
}

void IptuxAppIndicator::SetUnreadCount(int i) {
  if(i > 0) {
    app_indicator_set_status(priv->indicator, APP_INDICATOR_STATUS_ATTENTION);
  } else {
    app_indicator_set_status(priv->indicator, APP_INDICATOR_STATUS_ACTIVE);
  }
}

}  // namespace iptux
