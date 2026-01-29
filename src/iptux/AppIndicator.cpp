#include "config.h"
#include "AppIndicator.h"

#include <glib/gi18n.h>
#include <libayatana-appindicator/app-indicator.h>

namespace iptux {

class IptuxAppIndicatorPrivate {
 public:
  IptuxAppIndicatorPrivate(IptuxAppIndicator* owner) : owner(owner) {}
  ~IptuxAppIndicatorPrivate() {
    if (indicator) {
      g_object_unref(indicator);
    }
    if (menuBuilder) {
      g_object_unref(menuBuilder);
    }
  }
  IptuxAppIndicator* owner;
  AppIndicator* indicator;
  GtkBuilder* menuBuilder;

  static void onScrollEvent(IptuxAppIndicatorPrivate* self) {
    self->owner->sigActivateMainWindow.emit();
  }
};

IptuxAppIndicator::IptuxAppIndicator(GActionGroup* action_group) {
  this->priv = std::make_shared<IptuxAppIndicatorPrivate>(this);

  // app_indicator_new is deprecated since 0.5.94, and prefer
  // libayatana-appindicator-glib-dev, but unfortunately it's still not stable
  // enough.
  G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  priv->indicator = app_indicator_new("io.github.iptux_src.iptux", "iptux-icon",
                                      APP_INDICATOR_CATEGORY_COMMUNICATIONS);
  G_GNUC_END_IGNORE_DEPRECATIONS

  app_indicator_set_status(priv->indicator, APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_attention_icon_full(priv->indicator, "iptux-attention",
                                        "iptux-attention");

  app_indicator_set_title(priv->indicator, _("Iptux"));

  priv->menuBuilder =
      gtk_builder_new_from_resource(IPTUX_RESOURCE "gtk/AppIndicator.ui");

  GtkMenu* menu = GTK_MENU(gtk_menu_new_from_model(G_MENU_MODEL(
      gtk_builder_get_object(priv->menuBuilder, "app-indicator-menu"))));
  gtk_widget_insert_action_group(GTK_WIDGET(menu), "app", action_group);
  app_indicator_set_menu(priv->indicator, menu);

  g_signal_connect_swapped(priv->indicator, "scroll-event",
                           G_CALLBACK(IptuxAppIndicatorPrivate::onScrollEvent),
                           priv.get());
}

void IptuxAppIndicator::SetUnreadCount(int i) {
  if (i > 0) {
    app_indicator_set_status(priv->indicator, APP_INDICATOR_STATUS_ATTENTION);
  } else {
    app_indicator_set_status(priv->indicator, APP_INDICATOR_STATUS_ACTIVE);
  }
}

}  // namespace iptux
