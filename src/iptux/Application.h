#ifndef IPTUX_APPLICATION_H
#define IPTUX_APPLICATION_H

#include <gtk/gtk.h>

#include "iptux-core/IptuxConfig.h"
#include "iptux/UiProgramData.h"
#include "iptux/UiCoreThread.h"
#include "ShareFile.h"

namespace iptux {

class MainWindow;

class Application {
 public:
  explicit Application(std::shared_ptr<IptuxConfig> config);
  ~Application();

  int run(int argc, char** argv);

  GtkApplication* getApp() {return app;}
  std::shared_ptr<IptuxConfig> getConfig() { return config; }
  TransModel* getTransModel() { return transModel; }

private:
  std::shared_ptr<IptuxConfig> config;
  std::shared_ptr<UiProgramData> data;
  std::shared_ptr<UiCoreThread> cthrd;

  GtkApplication* app;
  MainWindow* window;
  ShareFile* shareFile;
  TransModel* transModel;
  bool started {false};

public:
  // for test
  void startup();
  void activate();

private:
  static void onStartup (Application& self);
  static void onActivate (Application& self);
  static void onQuit (void *, void *, Application& self);
  static void onPreferences (void *, void *, Application& self);
  static void onToolsTransmission (void *, void *, Application& self);
  static void onToolsSharedManagement (void *, void *, Application& self);
};

}

#endif
