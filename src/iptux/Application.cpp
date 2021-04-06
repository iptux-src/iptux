#include "config.h"
#include "Application.h"

#include <glib/gi18n.h>
#include <glog/logging.h>
#include <sys/stat.h>

#include "iptux-core/Exception.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"
#include "iptux/DataSettings.h"
#include "iptux/HelpDialog.h"
#include "iptux/IptuxResource.h"
#include "iptux/LogSystem.h"
#include "iptux/MainWindow.h"
#include "iptux/ShareFile.h"
#include "iptux/TransWindow.h"
#include "iptux/UiCoreThread.h"
#include "iptux/UiHelper.h"
#include "iptux/UiProgramData.h"
#include "iptux/dialog.h"
#include "iptux/global.h"

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

void iptux_init(LogSystem* logSystem) {
  signal(SIGPIPE, SIG_IGN);
  logSystem->systemLog("%s", _("Loading the process successfully!"));
}

void init_theme() {
  auto theme = gtk_icon_theme_get_default();
  gtk_icon_theme_prepend_search_path(theme, __PIXMAPS_PATH "/icon");
  gtk_icon_theme_prepend_search_path(theme, __PIXMAPS_PATH "/menu");
  gtk_icon_theme_prepend_search_path(theme, __PIXMAPS_PATH "/tip");
}
}  // namespace

Application::Application(shared_ptr<IptuxConfig> config)
    : config(config), data(nullptr), window(nullptr), shareFile(nullptr) {
  auto application_id =
      config->GetString("debug_application_id", "io.github.iptux-src.iptux");

  transModel = transModelNew();
  menuBuilder = nullptr;
  eventAdaptor = nullptr;
  logSystem = nullptr;

  app = gtk_application_new(application_id.c_str(), G_APPLICATION_FLAGS_NONE);
  g_signal_connect_swapped(app, "startup", G_CALLBACK(onStartup), this);
  g_signal_connect_swapped(app, "activate", G_CALLBACK(onActivate), this);

#if SYSTEM_DARWIN
  notificationService = new TerminalNotifierNoticationService();
#else
  notificationService = new GioNotificationService();
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
  g_object_unref(menuBuilder);
  transModelDelete(transModel);
  if (eventAdaptor) {
    delete eventAdaptor;
  }
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
  self.data = make_shared<UiProgramData>(self.config);
  self.logSystem = new LogSystem(self.data);
  self.cthrd = make_shared<UiCoreThread>(&self, self.data);
  g_cthrd = self.cthrd.get();
  self.window = new MainWindow(&self, *g_cthrd);
  self.eventAdaptor = new EventAdaptor(
      self.cthrd->signalEvent,
      [&](shared_ptr<const Event> event) { self.onEvent(event); });

  init_theme();

  iptux_register_resource();
  GActionEntry app_entries[] = {
      {"quit", G_ACTION_CALLBACK(onQuit), NULL, NULL, NULL, {0, 0, 0}},
      {"preferences",
       G_ACTION_CALLBACK(onPreferences),
       NULL,
       NULL,
       NULL,
       {0, 0, 0}},
      {"help.report_bug",
       G_ACTION_CALLBACK(onReportBug),
       NULL,
       NULL,
       NULL,
       {0, 0, 0}},
      {"tools.transmission",
       G_ACTION_CALLBACK(onToolsTransmission),
       NULL,
       NULL,
       NULL,
       {0, 0, 0}},
      {"tools.shared_management",
       G_ACTION_CALLBACK(onToolsSharedManagement),
       NULL,
       NULL,
       NULL,
       {0, 0, 0}},
      {"tools.open_chat_log",
       G_ACTION_CALLBACK(onOpenChatLog),
       NULL,
       NULL,
       NULL,
       {0, 0, 0}},
      {"tools.open_system_log",
       G_ACTION_CALLBACK(onOpenSystemLog),
       NULL,
       NULL,
       NULL,
       {0, 0, 0}},
      {"trans_model.changed"},
      {"trans_model.clear", G_ACTION_CALLBACK(onTransModelClear)},
      {"about", G_ACTION_CALLBACK(onAbout)},
  };

  g_action_map_add_action_entries(G_ACTION_MAP(self.app), app_entries,
                                  G_N_ELEMENTS(app_entries), &self);
  self.menuBuilder =
      gtk_builder_new_from_resource(IPTUX_RESOURCE "gtk/menus.ui");
  auto app_menu =
      G_MENU_MODEL(gtk_builder_get_object(self.menuBuilder, "appmenu"));
  gtk_application_set_app_menu(GTK_APPLICATION(self.app), app_menu);
  auto menubar =
      G_MENU_MODEL(gtk_builder_get_object(self.menuBuilder, "menubar"));
  gtk_application_set_menubar(GTK_APPLICATION(self.app), menubar);

  add_accelerator(self.app, "app.quit", "<Primary>Q");
  add_accelerator(self.app, "win.refresh", "F5");
  add_accelerator(self.app, "win.detect", "<Primary>D");
  add_accelerator(self.app, "win.find", "<Primary>F");
  add_accelerator(self.app, "win.attach_file", "<Ctrl>S");
  add_accelerator(self.app, "win.attach_folder", "<Ctrl>D");
  add_accelerator(self.app, "win.request_shared_resources", "<Ctrl>R");
  add_accelerator(self.app, "win.close", "<Primary>W");
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

  self.window->CreateWindow();
  try {
    g_cthrd->start();
  } catch (const Exception& e) {
    pop_warning(self.window->getWindow(), "%s", e.what());
    exit(1);
  }
  iptux_init(self.logSystem);
}

void Application::onQuit(void*, void*, Application& self) {
  if (self.window->isTransmissionActive()) {
    if (!pop_request_quit(GTK_WINDOW(self.window->getWindow()))) {
      return;
    }
  }
  g_application_quit(G_APPLICATION(self.app));
}

void Application::onPreferences(void*, void*, Application& self) {
  DataSettings::ResetDataEntry(GTK_WIDGET(self.window->getWindow()));
}

void Application::onToolsTransmission(void*, void*, Application& self) {
  self.openTransWindow();
}

void Application::onToolsSharedManagement(void*, void*, Application& self) {
  if (!self.shareFile) {
    self.shareFile = share_file_new(GTK_WINDOW(self.window->getWindow()));
  }
  share_file_run(self.shareFile);
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

void Application::onAbout(void*, void*, Application& self) {
  HelpDialog::AboutEntry(GTK_WINDOW(self.window->getWindow()));
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
    notificationService->sendNotification(
        G_APPLICATION(app), "iptux-new-message", title, summary,
        G_NOTIFICATION_PRIORITY_NORMAL, nullptr);
  }
  if (type == EventType::NEW_SHARE_FILE_FROM_FRIEND) {
    const NewShareFileFromFriendEvent* event =
        dynamic_cast<const NewShareFileFromFriendEvent*>(_event.get());
    auto title = stringFormat(_("New File from %s"),
                              event->GetFileInfo().fileown->getName().c_str());
    auto summary = event->GetFileInfo().filepath;
    notificationService->sendNotification(
        G_APPLICATION(app), "iptux-new-file", title, summary,
        G_NOTIFICATION_PRIORITY_NORMAL, nullptr);
  }
  if (type == EventType::RECV_FILE_FINISHED) {
    auto event = dynamic_pointer_cast<const RecvFileFinishedEvent>(_event);
    auto title = _("Receiveing File Finished");
    auto taskStat = getCoreThread()->GetTransTaskStat(event->GetTaskId());
    string summary;
    if (!taskStat) {
      summary = _("file info no longer exist");
    } else {
      summary = taskStat->getFilename();
    }
    notificationService->sendNotification(
        G_APPLICATION(app), "iptux-recv-file-finished", title, summary,
        G_NOTIFICATION_PRIORITY_NORMAL, nullptr);
  }
  if (type == EventType::CONFIG_CHANGED) {
    this->onConfigChanged();
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

}  // namespace iptux
