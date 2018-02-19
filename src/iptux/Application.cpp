#include "Application.h"
#include "iptux/ProgramData.h"
#include "iptux/global.h"
#include "iptux/ipmsg.h"
#include "iptux/support.h"
#include "iptux/StatusIcon.h"
#include "HelpDialog.h"
#include "DataSettings.h"
#include "ShareFile.h"
#include "output.h"
#include "dialog.h"
#include "iptux/IptuxResource.h"
#include "iptux/UiHelper.h"

typedef void (* GActionCallback) (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       user_data) ;
#define	G_ACTION_CALLBACK(f)			 ((GActionCallback) (f))

namespace iptux {

Application::Application(IptuxConfig& config)
: config(config),
  shareFile(nullptr) {
  data = new ProgramData(config);
  app = gtk_application_new ("io.github.iptux-src.iptux", G_APPLICATION_FLAGS_NONE);
  g_signal_connect_swapped(app, "startup", G_CALLBACK(onStartup), this);
  g_signal_connect_swapped(app, "activate", G_CALLBACK(onActivate), this);
  g_signal_connect_swapped(app, "notify::active-window", G_CALLBACK(onActiveWindowChanged), this);
  window = new MainWindow(app, config, *data);

  g_progdt = data;
  g_mwin = window;
}

Application::~Application() {
  g_object_unref(app);
  delete data;
  delete window;
}

int Application::run(int argc, char** argv) {
  return g_application_run (G_APPLICATION (app), argc, argv);
}

void Application::onStartup(Application& self) {
  iptux_register_resource();
  GActionEntry app_entries[] =  {
      { "quit", G_ACTION_CALLBACK(onQuit), NULL, NULL, NULL },
      { "preferences", G_ACTION_CALLBACK(onPreferences), NULL, NULL, NULL},
      { "help.faq", G_ACTION_CALLBACK(HelpDialog::onFaq), NULL, NULL, NULL },
      { "tools.transmission", G_ACTION_CALLBACK(onToolsTransmission), NULL, NULL, NULL },
      { "tools.shared_management", G_ACTION_CALLBACK(onToolsSharedManagement), NULL, NULL, NULL },
  };

  g_action_map_add_action_entries (G_ACTION_MAP (self.app),
                                   app_entries, G_N_ELEMENTS (app_entries),
                                   &self);
  auto builder = gtk_builder_new_from_resource(IPTUX_RESOURCE "gtk/menus.ui");
  auto app_menu = G_MENU_MODEL (gtk_builder_get_object (builder, "appmenu"));
  gtk_application_set_app_menu (GTK_APPLICATION (self.app), app_menu);
  auto menubar = G_MENU_MODEL (gtk_builder_get_object (builder, "menubar"));
  gtk_application_set_menubar(GTK_APPLICATION(self.app), menubar);
  g_object_unref (builder);
}

void Application::onActivate(Application& self) {
  g_cthrd = new CoreThread(self.config);
  StatusIcon* sicon = new StatusIcon(self.config, *self.window);
  g_sndsys = new SoundSystem();
  g_lgsys = new LogSystem(*self.data);

  self.window->SetStatusIcon(sicon);

  int port = self.config.GetInt("port", IPTUX_DEFAULT_PORT);
  self.window->CreateWindow();
  try {
    iptux_init(port);
  } catch (const BindFailedException& e) {
    pop_warning(self.window->getWindow(), "%s", e.what());
    exit(1);
  }
  sicon->CreateStatusIcon();
  g_cthrd->CoreThreadEntry();
}

void Application::onActiveWindowChanged(Application &self) {
  LOG_WARN("new active window %p", gtk_application_get_active_window(self.app));
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
}
