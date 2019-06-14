#ifndef IPTUX_UI_CALLBACKS_H
#define IPTUX_UI_CALLBACKS_H

#include <gtk/gtk.h>

extern "C" {
  G_MODULE_EXPORT
  gboolean iptux_on_activate_link(GtkAboutDialog *label, gchar *uri, gpointer user_data);
}

#endif
