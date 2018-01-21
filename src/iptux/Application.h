#ifndef IPTUX_APPLICATION_H
#define IPTUX_APPLICATION_H

#include <gtk/gtk.h>
#include "iptux/IptuxConfig.h"

namespace iptux {

class Application {
 public:
  explicit Application(IptuxConfig& config);
  ~Application();

  int run(int argc, char** argv);

private:
  IptuxConfig& config;
  GtkApplication* app;

private:
  static void onStartup (Application& self);
  static void onActivate (Application& self);
  static void onQuit (GSimpleAction *action,
                      GVariant      *parameter,
                      gpointer       user_data);
};

}

#endif
