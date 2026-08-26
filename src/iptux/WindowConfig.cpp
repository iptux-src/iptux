#include "config.h"
#include "WindowConfig.h"

using namespace std;

namespace iptux {

void WindowConfig::LoadFromConfig(shared_ptr<IptuxConfig> config) {
  int width = config->GetInt(prefix + "_width");
  if (width != 0) {
    this->width = width;
  }

  int height = config->GetInt(prefix + "_height");
  if (height != 0) {
    this->height = height;
  }
}

void WindowConfig::SaveToConfig(shared_ptr<IptuxConfig> config) {
  config->SetInt(prefix + "_width", width);
  config->SetInt(prefix + "_height", height);
  config->Save();
}

void WindowConfig::Save() {
  this->SaveToConfig(this->config);
}

gboolean WindowConfig::on_configure_event(GtkWidget* window,
                                          GdkEventConfigure*,
                                          gpointer user_data) {
  WindowConfig* self = (WindowConfig*)user_data;
  GdkWindow* gdk_win = gtk_widget_get_window(GTK_WIDGET(window));
  gint width;
  gint height;

  if (gdk_win != NULL) {
    GdkWindowState state = gdk_window_get_state(gdk_win);

    if (state & (GDK_WINDOW_STATE_MAXIMIZED | GDK_WINDOW_STATE_FULLSCREEN |
                 GDK_WINDOW_STATE_ICONIFIED)) {
      return FALSE;
    }
  }

  gtk_window_get_size(GTK_WINDOW(window), &width, &height);
  self->SetWidth(width).SetHeight(height).Save();
  return FALSE;
}

}  // namespace iptux
