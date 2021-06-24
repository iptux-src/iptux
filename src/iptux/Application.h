#ifndef IPTUX_APPLICATION_H
#define IPTUX_APPLICATION_H

#include <gtk/gtk.h>

#include "iptux-core/IptuxConfig.h"
#include "iptux-core/Models.h"
#include "iptux/EventAdaptor.h"
#include "iptux/NotificationService.h"
#include "iptux/UiModels.h"
#include "iptux/UiProgramData.h"

namespace iptux {

class MainWindow;
class UiCoreThread;
typedef GtkWindow TransWindow;
typedef GtkDialog ShareFile;

class Application {
 public:
  explicit Application(std::shared_ptr<IptuxConfig> config);
  ~Application();

  int run(int argc, char** argv);

  void openTransWindow();

  GtkApplication* getApp() { return app; }
  std::shared_ptr<IptuxConfig> getConfig() { return config; }
  TransModel* getTransModel() { return transModel; }
  MainWindow* getMainWindow() { return window; }
  GtkBuilder* getMenuBuilder() { return menuBuilder; }
  LogSystem* getLogSystem() { return logSystem; }
  std::shared_ptr<UiProgramData> getProgramData() { return data; }
  std::shared_ptr<UiCoreThread> getCoreThread() { return cthrd; }
  void refreshTransTasks();
  PPalInfo getMe();

 private:
  std::shared_ptr<IptuxConfig> config;
  std::shared_ptr<UiProgramData> data;
  std::shared_ptr<UiCoreThread> cthrd;

  GtkApplication* app;
  GtkBuilder* menuBuilder;

  TransModel* transModel;

  MainWindow* window = 0;
  ShareFile* shareFile = 0;
  TransWindow* transWindow = 0;
  EventAdaptor* eventAdaptor = 0;
  LogSystem* logSystem = 0;
  NotificationService* notificationService = 0;
  bool started{false};

 public:
  // for test
  void startup();
  void activate();

 private:
  void onEvent(std::shared_ptr<const Event> event);
  void onConfigChanged();
  void updateItemToTransTree(const TransFileModel& para);
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
  static void onOpenChat(GSimpleAction* action,
                         GVariant* value,
                         Application& self);
  static gboolean onTransWindowDelete(Application& self);
};

}  // namespace iptux

#endif
