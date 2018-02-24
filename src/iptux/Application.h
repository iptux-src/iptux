#ifndef IPTUX_APPLICATION_H
#define IPTUX_APPLICATION_H

#include <gtk/gtk.h>

#include "iptux/IptuxConfig.h"
#include "iptux/UiProgramData.h"
#include "MainWindow.h"
#include "ShareFile.h"

namespace iptux {

class Application {
 public:
  explicit Application(IptuxConfig& config);
  ~Application();

  int run(int argc, char** argv);

private:
  IptuxConfig& config;
  UiProgramData* data;

  GtkApplication* app;
  MainWindow* window;
  ShareFile* shareFile;

private:
  static void onStartup (Application& self);
  static void onActivate (Application& self);
  static void onActiveWindowChanged (Application& self);
  static void onQuit (void *, void *, Application& self);
  static void onPreferences (void *, void *, Application& self);
  static void onToolsTransmission (void *, void *, Application& self);
  static void onToolsSharedManagement (void *, void *, Application& self);
};

}

#endif
