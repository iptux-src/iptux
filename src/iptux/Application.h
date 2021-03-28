#ifndef IPTUX_APPLICATION_H
#define IPTUX_APPLICATION_H

#include <gtk/gtk.h>

#include "iptux-core/IptuxConfig.h"
#include "iptux/EventAdaptor.h"
#include "iptux/ShareFile.h"
#include "iptux/UiModels.h"
#include "iptux/UiProgramData.h"

namespace iptux {

class MainWindow;
class UiCoreThread;

class Application {
 public:
  explicit Application(std::shared_ptr<IptuxConfig> config);
  ~Application();

  int run(int argc, char** argv);

  GtkApplication* getApp() { return app; }
  std::shared_ptr<IptuxConfig> getConfig() { return config; }
  TransModel* getTransModel() { return transModel; }
  MainWindow* getMainWindow() { return window; }
  GtkBuilder* getMenuBuilder() { return menuBuilder; }
  std::shared_ptr<UiProgramData> getProgramData() { return data; }
  std::shared_ptr<UiCoreThread> getCoreThread() { return cthrd; }
  void refreshTransTasks();

 private:
  std::shared_ptr<IptuxConfig> config;
  std::shared_ptr<UiProgramData> data;
  std::shared_ptr<UiCoreThread> cthrd;

  GtkApplication* app;
  MainWindow* window;
  GtkBuilder* menuBuilder;
  ShareFile* shareFile;
  TransModel* transModel;
  EventAdaptor* eventAdaptor;
  bool started{false};

 public:
  // for test
  void startup();
  void activate();

 private:
  void onEvent(std::shared_ptr<const Event> event);
  static void onAbout(void*, void*, Application& self);
  static void onActivate(Application& self);
  static void onPreferences(void*, void*, Application& self);
  static void onQuit(void*, void*, Application& self);
  static void onStartup(Application& self);
  static void onToolsSharedManagement(void*, void*, Application& self);
  static void onToolsTransmission(void*, void*, Application& self);
  static void onOpenChatLog(void*, void*, Application& self);
  static void onOpenSystemLog(void*, void*, Application& self);
  static void onTransModelClear(void*, void*, Application& self);
};

}  // namespace iptux

#endif
