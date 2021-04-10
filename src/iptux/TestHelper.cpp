#include "config.h"

#include "iptux/TestHelper.h"

namespace iptux {

GtkBuilder* newTestGtkBuilder() {
  return gtk_builder_new_from_resource(IPTUX_RESOURCE "gtk/main.ui");
}

}  // namespace iptux
