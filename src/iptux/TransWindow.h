#ifndef IPTUX_TRANSWINDOW_H
#define IPTUX_TRANSWINDOW_H

#include <gtk/gtk.h>

#include "iptux/Application.h"

namespace iptux {

typedef GtkWindow TransWindow;
TransWindow* trans_window_new(Application* app, GtkWindow* parent);

}

#endif //IPTUX_TRANSWINDOW_H
