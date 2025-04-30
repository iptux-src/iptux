#include "config.h"
#include "Application.h"

#include <glib/gi18n.h>
#include <sys/stat.h>

#include "iptux-core/Exception.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"
#include "iptux/AboutDialog.h"
#include "iptux/AppIndicator.h"
#include "iptux/DataSettings.h"
#include "iptux/DialogPeer.h"
#include "iptux/IptuxResource.h"
#include "iptux/LogSystem.h"
#include "iptux/MainWindow.h"
#include "iptux/ShareFile.h"
#include "iptux/TransWindow.h"
#include "iptux/UiCoreThread.h"
#include "iptux/UiHelper.h"
#include "iptux/dialog.h"

#if SYSTEM_DARWIN
#include "iptux/TerminalNotifierNotificationService.h"
#else
#include "iptux/GioNotificationService.h"
#endif

#if SYSTEM_DARWIN
#include "iptux/Darwin.h"
#endif

using namespace std;

typedef void (*GActionCallback)(GSimpleAction* action,
                                GVariant* parameter,
                                gpointer user_data);
#define G_ACTION_CALLBACK(f) ((GActionCallback)(f))

namespace iptux {

namespace {
void onReportBug() {
  iptux_open_url("https://github.com/iptux-src/iptux/issues/new");
}

void onWhatsNew() {
  iptux_open_url("https://github.com/iptux-src/iptux/blob/master/NEWS");
}

void iptux_init(LogSystem* logSystem) {
  signal(SIGPIPE, SIG_IGN);
  logSystem->systemLog("%s", _("Loading the process successfully!"));
}

void init_theme(Application* app) {
  auto theme = gtk_icon_theme_get_default();
  gtk_icon_theme_prepend_search_path(theme, __PIXMAPS_PATH "/icon");
  gtk_icon_theme_prepend_search_path(theme, __PIXMAPS_PATH "/menu");
  gtk_icon_theme_prepend_search_path(theme, __PIXMAPS_PATH "/tip");
  gtk_icon_theme_prepend_search_path(
      theme, app->getCoreThread()->getUserIconPath().c_str());
}
}  // namespace

Application::Application(shared_ptr<IptuxConfig> config)
    : config(config), data(nullptr), window(nullptr), shareFile(nullptr) {
  auto application_id =
      config->GetString("debug_application_id", "io.github.iptux_src.iptux");

  transModel = transModelNew();
  menuBuilder = nullptr;
  logSystem = nullptr;

  app = gtk_application_new(application_id.c_str(), G_APPLICATION_FLAGS_NONE);
  g_signal_connect_swapped(app, "startup", G_CALLBACK(onStartup), this);
  g_signal_connect_swapped(app, "activate", G_CALLBACK(onActivate), this);

#if SYSTEM_DARWIN
  notificationService = new TerminalNotifierNoticationService();
#else
  notificationService = new GioNotificationService();
  use_header_bar_ = true;
  // GError* error = nullptr;
  // if(!g_application_register(G_APPLICATION(app), nullptr, &error)) {
  //   LOG_WARN("g_application_register failed: %s-%d-%s",
  //   g_quark_to_string(error->domain),
  //     error->code, error->message);
  // }
#endif
}

Application::~Application() {
  g_object_unref(app);
  if (menuBuilder) {
    g_object_unref(menuBuilder);
  }
  transModelDelete(transModel);
  if (logSystem) {
    delete logSystem;
  }
  delete window;
  delete notificationService;
}

int Application::run(int argc, char** argv) {
  return g_application_run(G_APPLICATION(app), argc, argv);
}

void Application::startup() {
  Application::onStartup(*this);
}

void Application::activate() {
  Application::onActivate(*this);
}

void Application::onStartup(Application& self) {
  init_theme(&self);
  iptux_register_resource();
  self.menuBuilder =
      gtk_builder_new_from_resource(IPTUX_RESOURCE "gtk/menus.ui");
  self.data = make_shared<ProgramData>(self.config);
  self.logSystem = new LogSystem(self.data);
  self.cthrd = make_shared<UiCoreThread>(&self, self.data);
  if (self.enable_app_indicator_) {
    self.app_indicator = make_shared<IptuxAppIndicator>(&self);
  }

  bool use_app_menu = true;
#if SYSTEM_DARWIN
#else
  use_app_menu = gtk_application_prefers_app_menu(self.app);
#endif

  if (use_app_menu) {
    auto app_menu =
        G_MENU_MODEL(gtk_builder_get_object(self.menuBuilder, "appmenu"));
    gtk_application_set_app_menu(GTK_APPLICATION(self.app), app_menu);
    self.menu_ = G_MENU_MODEL(
        gtk_builder_get_object(self.menuBuilder, "menubar-when-app-menu"));
    if (!self.use_header_bar()) {
      gtk_application_set_menubar(GTK_APPLICATION(self.app), self.menu());
    }
  } else {
    self.menu_ = G_MENU_MODEL(
        gtk_builder_get_object(self.menuBuilder, "menubar-when-no-app-menu"));
    if (!self.use_header_bar()) {
      gtk_application_set_menubar(GTK_APPLICATION(self.app), self.menu());
    }
  }

  self.window = new MainWindow(&self, *self.cthrd);

  GActionEntry app_entries[] = {
      makeActionEntry("quit", G_ACTION_CALLBACK(onQuit)),
      makeActionEntry("preferences", G_ACTION_CALLBACK(onPreferences)),
      makeActionEntry("help.report_bug", G_ACTION_CALLBACK(onReportBug)),
      makeActionEntry("help.whats_new", G_ACTION_CALLBACK(onWhatsNew)),
      makeActionEntry("tools.transmission",
                      G_ACTION_CALLBACK(onToolsTransmission)),
      makeActionEntry("tools.shared_management",
                      G_ACTION_CALLBACK(onToolsSharedManagement)),
      makeActionEntry("tools.open_chat_log", G_ACTION_CALLBACK(onOpenChatLog)),
      makeActionEntry("tools.open_system_log",
                      G_ACTION_CALLBACK(onOpenSystemLog)),
      makeActionEntry("trans_model.changed", nullptr),
      makeActionEntry("trans_model.clear",
                      G_ACTION_CALLBACK(onTransModelClear)),
      makeActionEntry("about", G_ACTION_CALLBACK(onAbout)),
      makeParamActionEntry("open-chat", G_ACTION_CALLBACK(onOpenChat), "s"),
      makeActionEntry("window.close", G_ACTION_CALLBACK(onWindowClose)),
      makeActionEntry("open_main_window", G_ACTION_CALLBACK(onOpenMainWindow)),
  };

  g_action_map_add_action_entries(G_ACTION_MAP(self.app), app_entries,
                                  G_N_ELEMENTS(app_entries), &self);

  add_accelerator(self.app, "app.quit", "<Primary>Q");
  add_accelerator(self.app, "win.refresh", "F5");
  add_accelerator(self.app, "win.detect", "<Primary>D");
  add_accelerator(self.app, "win.find", "<Primary>F");
  add_accelerator(self.app, "win.attach_file", "<Ctrl>S");
  add_accelerator(self.app, "win.attach_folder", "<Ctrl>D");
  add_accelerator(self.app, "win.request_shared_resources", "<Ctrl>R");
  add_accelerator(self.app, "app.window.close", "<Primary>W");
  self.onConfigChanged();

#if SYSTEM_DARWIN
  install_darwin_icon();
#endif
}

void Application::onActivate(Application& self) {
  if (self.started) {
    return;
  }
  self.started = true;

  try {
    self.cthrd->start();
  } catch (const Exception& e) {
    pop_warning(self.window->getWindow(), "%s", e.what());
    exit(1);
  }
  iptux_init(self.logSystem);
  g_idle_add(G_SOURCE_FUNC(Application::ProcessEvents), &self);
}

void Application::onQuit(void*, void*, Application& self) {
  if (!transModelIsFinished(self.transModel)) {
    if (!pop_request_quit(GTK_WINDOW(self.window->getWindow()))) {
      return;
    }
  }
  g_application_quit(G_APPLICATION(self.app));
}

void Application::onPreferences(void*, void*, Application& self) {
  DataSettings::ResetDataEntry(&self, GTK_WIDGET(self.window->getWindow()));
}

void Application::onOpenMainWindow(void*, void*, Application& self) {
  self.getMainWindow()->Show();
}

void Application::onToolsTransmission(void*, void*, Application& self) {
  self.openTransWindow();
}

void Application::onToolsSharedManagement(void*, void*, Application& self) {
  if (!self.shareFile) {
    self.shareFile = shareFileNew(&self);
  }
  shareFileRun(self.shareFile, GTK_WINDOW(self.window->getWindow()));
}

void Application::onOpenChatLog(void*, void*, Application& self) {
  auto path = self.getCoreThread()->getLogSystem()->getChatLogPath();
  iptux_open_url(path.c_str());
}

void Application::onOpenSystemLog(void*, void*, Application& self) {
  auto path = self.getCoreThread()->getLogSystem()->getSystemLogPath();
  iptux_open_url(path.c_str());
}

void Application::onTransModelClear(void*, void*, Application& self) {
  self.getCoreThread()->clearFinishedTransTasks();
}

void Application::onWindowClose(void*, void*, Application& self) {
  auto window = gtk_application_get_active_window(self.app);
  if (window) {
    gtk_window_close(window);
  }
}

void Application::onAbout(void*, void*, Application& self) {
  aboutDialogEntry(GTK_WINDOW(self.window->getWindow()));
}

void Application::refreshTransTasks() {
  auto transModels = getCoreThread()->listTransTasks();
  transModelLoadFromTransFileModels(transModel, transModels);
}

void Application::onEvent(shared_ptr<const Event> _event) {
  EventType type = _event->getType();
  if (type == EventType::NEW_MESSAGE) {
    const NewMessageEvent* event =
        dynamic_cast<const NewMessageEvent*>(_event.get());
    auto title = stringFormat(_("New Message from %s"),
                              event->getMsgPara().getPal()->getName().c_str());
    auto summary = event->getMsgPara().getSummary();
    auto action = stringFormat(
        "app.open-chat::%s",
        event->getMsgPara().getPal()->GetKey().GetIpv4String().c_str());
    notificationService->sendNotification(
        G_APPLICATION(app), "iptux-new-message", title, summary, action,
        G_NOTIFICATION_PRIORITY_NORMAL, nullptr);
  }
  if (type == EventType::NEW_SHARE_FILE_FROM_FRIEND) {
    const NewShareFileFromFriendEvent* event =
        dynamic_cast<const NewShareFileFromFriendEvent*>(_event.get());
    auto title = stringFormat(_("New File from %s"),
                              event->GetFileInfo().fileown->getName().c_str());
    auto summary = event->GetFileInfo().filepath;
    auto action = stringFormat(
        "app.open-chat::%s",
        event->GetPalKey().GetIpv4String().c_str());
    notificationService->sendNotification(
        G_APPLICATION(app), "iptux-new-file", title, summary, action,
        G_NOTIFICATION_PRIORITY_NORMAL, nullptr);
  }
  if (type == EventType::RECV_FILE_FINISHED) {
    auto event = dynamic_pointer_cast<const RecvFileFinishedEvent>(_event);
    auto title = _("Receiving File Finished");
    auto taskStat = getCoreThread()->GetTransTaskStat(event->GetTaskId());
    string summary;
    if (!taskStat) {
      summary = _("file info no longer exist");
    } else {
      summary = taskStat->getFilename();
    }
    notificationService->sendNotification(
        G_APPLICATION(app), "iptux-recv-file-finished", title, summary, "",
        G_NOTIFICATION_PRIORITY_NORMAL, nullptr);
  }
  if (type == EventType::CONFIG_CHANGED) {
    this->onConfigChanged();
  }
  if (type == EventType::SEND_FILE_STARTED ||
      type == EventType::RECV_FILE_STARTED) {
    auto event = dynamic_cast<const AbstractTaskIdEvent*>(_event.get());
    g_assert(event);
    auto taskId = event->GetTaskId();
    auto para = cthrd->GetTransTaskStat(taskId);
    if (!para.get()) {
      LOG_WARN("got task id %d, but no info in CoreThread", taskId);
      return;
    }
    this->updateItemToTransTree(*para);
    auto g_progdt = cthrd->getProgramData();
    if (g_progdt->IsAutoOpenFileTrans()) {
      this->openTransWindow();
    }
    return;
  }

  if (type == EventType::SEND_FILE_FINISHED ||
      type == EventType::RECV_FILE_FINISHED) {
    auto event = dynamic_cast<const AbstractTaskIdEvent*>(_event.get());
    g_assert(event);
    auto taskId = event->GetTaskId();
    auto para = cthrd->GetTransTaskStat(taskId);
    this->updateItemToTransTree(*para);
    return;
  }

  if (type == EventType::TRANS_TASKS_CHANGED) {
    this->refreshTransTasks();
    return;
  }
}

PPalInfo Application::getMe() {
  return this->getCoreThread()->getMe();
}

void Application::openTransWindow() {
  if (transWindow == nullptr) {
    transWindow = trans_window_new(this, GTK_WINDOW(window->getWindow()));
    gtk_widget_show_all(GTK_WIDGET(transWindow));
    gtk_widget_hide(GTK_WIDGET(transWindow));
    g_signal_connect_swapped(transWindow, "delete-event",
                             G_CALLBACK(onTransWindowDelete), this);
  }
  gtk_window_present(GTK_WINDOW(transWindow));
}

gboolean Application::onTransWindowDelete(iptux::Application& self) {
  self.transWindow = nullptr;
  return FALSE;
}

void Application::onConfigChanged() {
  if (getCoreThread()->getProgramData()->IsEnterSendMessage()) {
    add_accelerator(app, "win.send_message", "Return");
  } else {
    add_accelerator(app, "win.send_message", "<Primary>Return");
  }
}

void Application::updateItemToTransTree(const TransFileModel& para) {
  transModelUpdateFromTransFileModel(transModel, para);
  g_action_group_activate_action(G_ACTION_GROUP(this->getApp()),
                                 "trans_model.changed", nullptr);
}

void Application::onOpenChat(GSimpleAction*,
                             GVariant* value,
                             Application& self) {
  string ipv4 = g_variant_get_string(value, nullptr);
  auto pal = self.cthrd->GetPal(ipv4);
  if (!pal) {
    return;
  }
  auto groupInfo = self.cthrd->GetPalRegularItem(pal.get());
  if (!groupInfo) {
    return;
  }
  if (groupInfo->getDialog()) {
    gtk_window_present(GTK_WINDOW(groupInfo->getDialog()));
    return;
  }
  DialogPeer::PeerDialogEntry(&self, groupInfo);
}

gboolean Application::ProcessEvents(gpointer data) {
  auto self = static_cast<Application*>(data);
  if (self->getCoreThread()->HasEvent()) {
    auto start = chrono::high_resolution_clock::now();
    auto e = self->getCoreThread()->PopEvent();
    self->onEvent(e);
    self->getMainWindow()->ProcessEvent(e);
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    LOG_INFO(
        "type: %s, from: %s, time: %jdus", EventTypeToStr(e->getType()),
        e->getSource().c_str(),
        (intmax_t)chrono::duration_cast<chrono::microseconds>(elapsed).count());
    g_idle_add(Application::ProcessEvents, data);
  } else {
    g_timeout_add(100, Application::ProcessEvents, data);  // 100ms
  }
  return G_SOURCE_REMOVE;
}

}  // namespace iptux
