#ifndef IPTUX_UI_CALLBACKS_H
#define IPTUX_UI_CALLBACKS_H

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

extern "C" {
  G_MODULE_EXPORT
  gboolean iptux_on_activate_link(GtkAboutDialog *label, gchar *uri, gpointer user_data);

  G_MODULE_EXPORT
  void iptux_on_ipv4_entry_insert_text(GtkWidget *entry, gchar *text, gint length);

  G_MODULE_EXPORT
  gboolean iptux_on_detect_pal_ipv4_entry_escape(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
}

#endif
