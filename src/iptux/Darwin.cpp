#include "Darwin.h"

#include <glib/gi18n.h>
#include <gtkosxapplication.h>

using namespace std;


namespace iptux {
  void install_darwin_icon() {
    auto app = gtkosx_application_get();
    auto theme = gtk_icon_theme_get_default();
    GError *error = nullptr;
    auto pixbuf = gtk_icon_theme_load_icon(theme, "icon-bug", 64, GtkIconLookupFlags(0), &error);
    if(!pixbuf) {
      g_warning (_("Couldn’t load icon: %s"), error->message);
      g_error_free (error);
      return;
    }
    gtkosx_application_set_dock_icon_pixbuf(app, pixbuf);
    g_object_unref(pixbuf);
  }
}
