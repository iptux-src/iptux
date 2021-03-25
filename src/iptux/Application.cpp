#include "config.h"
#include "Application.h"

#include <sys/stat.h>
#include <glib/gi18n.h>
#include <glog/logging.h>

#include "iptux-core/Exception.h"
#include "iptux-utils/output.h"
#include "iptux/DataSettings.h"
#include "iptux/dialog.h"
#include "iptux/global.h"
#include "iptux/IptuxResource.h"
#include "iptux/ShareFile.h"
#include "iptux/StatusIcon.h"
#include "iptux/UiHelper.h"
#include "iptux/UiProgramData.h"
#include "iptux/HelpDialog.h"
#include "iptux/UiCoreThread.h"
#include "iptux/LogSystem.h"

#if SYSTEM_DARWIN
#include "iptux/Darwin.h"
#endif

using namespace std;

typedef void (* GActionCallback) (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       user_data) ;
#define	G_ACTION_CALLBACK(f)			   ((GActionCallback) (f))

namespace iptux {

namespace {
  void onReportBug() {
    iptux_open_url("https://github.com/iptux-src/iptux/issues/new");
  }

  void iptux_init() {
    g_sndsys->InitSublayer();

    signal(SIGPIPE, SIG_IGN);
    g_cthrd->SystemLog("%s", _("Loading the process successfully!"));
  }

  void init_theme() {
    auto theme = gtk_icon_theme_get_default();
    gtk_icon_theme_prepend_search_path(theme, __PIXMAPS_PATH "/icon");
    gtk_icon_theme_prepend_search_path(theme, __PIXMAPS_PATH "/menu");
    gtk_icon_theme_prepend_search_path(theme, __PIXMAPS_PATH "/tip");
  }
}

Application::Application(shared_ptr<IptuxConfig> config)
: config(config),
  data(nullptr),
  window(nullptr),
  shareFile(nullptr)
{
  auto application_id = config->GetString("debug_application_id", "io.github.iptux-src.iptux");
  app = gtk_application_new (application_id.c_str(), G_APPLICATION_FLAGS_NONE);
  g_signal_connect_swapped(app, "startup", G_CALLBACK(onStartup), this);
  g_signal_connect_swapped(app, "activate", G_CALLBACK(onActivate), this);

  transModel = transModelNew();
  menuBuilder = nullptr;
}

Application::~Application() {
  g_object_unref(app);
  g_object_unref(menuBuilder);
  transModelDelete(transModel);
  delete window;
}

int Application::run(int argc, char** argv) {
  return g_application_run (G_APPLICATION (app), argc, argv);
}

void Application::startup() {
  Application::onStartup(*this);
}

void Application::activate() {
  Application::onActivate(*this);
}

void Application::onStartup(Application& self) {
  self.data = make_shared<UiProgramData>(self.config);
  self.cthrd = make_shared<UiCoreThread>(&self, self.data);
  g_cthrd = self.cthrd.get();
  self.window = new MainWindow(&self, *g_cthrd);
  g_mwin = self.window;

  init_theme();

  iptux_register_resource();
  GActionEntry app_entries[] =  {
      { "quit", G_ACTION_CALLBACK(onQuit), NULL, NULL, NULL, {0,0,0}},
      { "preferences", G_ACTION_CALLBACK(onPreferences), NULL, NULL, NULL, {0,0,0}},
      { "help.report_bug", G_ACTION_CALLBACK(onReportBug), NULL, NULL, NULL, {0,0,0}},
      { "tools.transmission", G_ACTION_CALLBACK(onToolsTransmission), NULL, NULL, NULL, {0,0,0}},
      { "tools.shared_management", G_ACTION_CALLBACK(onToolsSharedManagement), NULL, NULL, NULL, {0,0,0}},
      { "tools.open_chat_log", G_ACTION_CALLBACK(onOpenChatLog), NULL, NULL, NULL, {0,0,0}},
      { "tools.open_system_log", G_ACTION_CALLBACK(onOpenSystemLog), NULL, NULL, NULL, {0,0,0}},
      { "trans_model.changed" },
      { "trans_model.clear", G_ACTION_CALLBACK(onTransModelClear)},
      { "about", G_ACTION_CALLBACK(onAbout)},
  };

  g_action_map_add_action_entries (G_ACTION_MAP (self.app),
                                   app_entries, G_N_ELEMENTS (app_entries),
                                   &self);
  self.menuBuilder = gtk_builder_new_from_resource(IPTUX_RESOURCE "gtk/menus.ui");
  auto app_menu = G_MENU_MODEL (gtk_builder_get_object (self.menuBuilder, "appmenu"));
  gtk_application_set_app_menu (GTK_APPLICATION (self.app), app_menu);
  auto menubar = G_MENU_MODEL (gtk_builder_get_object (self.menuBuilder, "menubar"));
  gtk_application_set_menubar(GTK_APPLICATION(self.app), menubar);

  add_accelerator(self.app, "app.quit", "<Primary>Q");
  add_accelerator(self.app, "win.refresh", "F5");
  add_accelerator(self.app, "win.detect", "<Primary>D");
  add_accelerator(self.app, "win.find", "<Primary>F");
  add_accelerator(self.app, "win.attach_file", "<Ctrl>S");
  add_accelerator(self.app, "win.attach_folder", "<Ctrl>D");
  add_accelerator(self.app, "win.request_shared_resources", "<Ctrl>R");
  add_accelerator(self.app, "win.close", "<Primary>W");
  add_accelerator(self.app, "win.send_message", "<Primary>Return");

#if SYSTEM_DARWIN
  install_darwin_icon();
#endif
}

void Application::onActivate(Application& self) {
  if(self.started) {
    return;
  }
  self.started = true;

  StatusIcon* sicon = new StatusIcon(self.config, *self.window);
  g_sndsys = new SoundSystem();

  self.window->SetStatusIcon(sicon);
  self.window->CreateWindow();
  try {
    g_cthrd->start();
  } catch (const Exception& e) {
    pop_warning(self.window->getWindow(), "%s", e.what());
    exit(1);
  }
  iptux_init();
  sicon->CreateStatusIcon();
}

void Application::onQuit (void*, void*, Application& self) {
  if(self.window->isTransmissionActive()) {
    if(!pop_request_quit(GTK_WINDOW(self.window->getWindow()))) {
      return;
    }
  }
  g_application_quit(G_APPLICATION (self.app));
}

void Application::onPreferences(void *, void *, Application &self) {
  DataSettings::ResetDataEntry(GTK_WIDGET(self.window->getWindow()));
}

void Application::onToolsTransmission(void *, void *, Application &self) {
  self.window->OpenTransWindow();
}

void Application::onToolsSharedManagement(void *, void *, Application &self) {
  if(!self.shareFile) {
    self.shareFile = share_file_new(GTK_WINDOW(self.window->getWindow()));
  }
  share_file_run(self.shareFile);
}

void Application::onOpenChatLog(void *, void *, Application &self) {
  auto path = self.getCoreThread()->getLogSystem()->getChatLogPath();
  iptux_open_url(path.c_str());
}

void Application::onOpenSystemLog(void *, void *, Application &self) {
  auto path = self.getCoreThread()->getLogSystem()->getSystemLogPath();
  iptux_open_url(path.c_str());
}

void Application::onTransModelClear(void *, void *, Application &self) {
  self.getCoreThread()->clearFinishedTransTasks();
}

void Application::onAbout(void*, void*, Application&self) {
  HelpDialog::AboutEntry(GTK_WINDOW(self.window->getWindow()));
}

void Application::refreshTransTasks() {
  auto transModels = getCoreThread()->listTransTasks();
  transModelLoadFromTransFileModels(transModel, transModels);
}

}
