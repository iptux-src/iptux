#ifndef IPTUX_TRANSWINDOW_H
#define IPTUX_TRANSWINDOW_H

#include <gtk/gtk.h>

namespace iptux {

typedef GtkWindow TransWindow;
TransWindow* trans_window_new(GtkWindow* parent);

}

#endif //IPTUX_TRANSWINDOW_H
