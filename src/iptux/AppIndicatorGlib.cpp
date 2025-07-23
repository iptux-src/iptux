#include "config.h"
#include "AppIndicator.h"

#include "iptux-utils/output.h"
#include <ayatana-appindicator.h>
#include <glib/gi18n.h>

#include "iptux/Application.h"
#include "iptux/UiCoreThread.h"
#include <gio/gio.h>

namespace iptux {

static void onItemActivate(GSimpleAction* action, GVariant*, gpointer ctx) {
  GActionGroup* action_group = G_ACTION_GROUP(ctx);
  const gchar* action_name = g_action_get_name(G_ACTION(action));
  g_action_group_activate_action(action_group, action_name, NULL);
}

static GSimpleActionGroup* create_proxy_action_group(
    GActionGroup* action_group) {
  GSimpleActionGroup* group = g_simple_action_group_new();
  const char* actionsp[] = {
      "open_main_window",
      "preferences",
      "quit",
  };

  for (const char* action_name : actionsp) {
    GSimpleAction* action = g_simple_action_new(action_name, NULL);
    g_action_map_add_action(G_ACTION_MAP(group), G_ACTION(action));
    g_signal_connect(action, "activate", G_CALLBACK(onItemActivate),
                     action_group);
  }
  return group;
}

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

  static void onScrollEvent(IptuxAppIndicatorPrivate* self) {
    g_action_group_activate_action(G_ACTION_GROUP(self->app->getApp()),
                                   "open_main_window", NULL);
  }
};

IptuxAppIndicator::IptuxAppIndicator(Application* app) {
  this->priv = std::make_shared<IptuxAppIndicatorPrivate>(app);
  priv->indicator = app_indicator_new("io.github.iptux_src.iptux", "iptux-icon",
                                      APP_INDICATOR_CATEGORY_COMMUNICATIONS);
  app_indicator_set_status(priv->indicator, APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_attention_icon(priv->indicator, "iptux-attention", "Iptux");

  app_indicator_set_title(priv->indicator, _("Iptux"));

  priv->menuBuilder =
      gtk_builder_new_from_resource(IPTUX_RESOURCE "gtk/AppIndicator.ui");

  app->getCoreThread()->sigUnreadMsgCountUpdated.connect(
      sigc::mem_fun(*this, &IptuxAppIndicator::SetUnreadCount));

  //   g_signal_connect_swapped(priv->indicator, "scroll-event",
  //                            G_CALLBACK(IptuxAppIndicatorPrivate::onScrollEvent),
  //                            priv.get());

  GMenu* menu =
      G_MENU(gtk_builder_get_object(priv->menuBuilder, "app-indicator-menu"));
  if (!menu) {
    LOG_WARN("Failed to get menu from AppIndicator UI resource");
    return;
  }
  LOG_INFO("AppIndicator menu loaded successfully: %d items",
           g_menu_model_get_n_items(G_MENU_MODEL(menu)));

  app_indicator_set_menu(priv->indicator, menu);
  app_indicator_set_actions(
      priv->indicator,
      create_proxy_action_group(G_ACTION_GROUP(app->getApp())));
}

void IptuxAppIndicator::SetUnreadCount(int i) {
  if (i > 0) {
    app_indicator_set_status(priv->indicator, APP_INDICATOR_STATUS_ATTENTION);
  } else {
    app_indicator_set_status(priv->indicator, APP_INDICATOR_STATUS_ACTIVE);
  }
}

}  // namespace iptux
