#include "config.h"
#include "UiCallbacks.h"

#include "iptux/UiHelper.h"

using namespace iptux;

gboolean iptux_on_activate_link(GtkAboutDialog *, gchar *uri, gpointer) {
  iptux_open_url(uri);
  return TRUE;
}


