#include "config.h"
#include "UiCallbacks.h"

#include "iptux-core/output.h"
#include "iptux/UiHelper.h"
#include "iptux/callback.h"

using namespace iptux;

gboolean iptux_on_activate_link(GtkAboutDialog *, gchar *uri, gpointer) {
  iptux_open_url(uri);
  return TRUE;
}

void iptux_on_ipv4_entry_insert_text(GtkWidget *entry, gchar *text, gint length) {
  entry_insert_numeric(entry, text, length);
}

gboolean iptux_on_detect_pal_ipv4_entry_escape(GtkWidget *widget, GdkEventKey *event, gpointer) {
  if(event->keyval == GDK_KEY_Escape) {
    gtk_dialog_response(GTK_DIALOG(gtk_widget_get_toplevel(widget)), GTK_RESPONSE_CLOSE);
  }
  return FALSE;
}




