#include "config.h"

#include "iptux/TestHelper.h"

namespace iptux {

GtkBuilder* newTestGtkBuilder() {
  auto builder = gtk_builder_new_from_file(__UI_PATH "/main.ui");
  gtk_builder_connect_signals(builder, nullptr);
  return builder;
}

}  // namespace iptux
