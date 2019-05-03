#include "config.h"
#include "Application.h"

#include "iptux/UiProgramData.h"
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
  data(nullptr),
  window(nullptr),
  shareFile(nullptr) {
  app = gtk_application_new ("io.github.iptux-src.iptux", G_APPLICATION_FLAGS_NONE);
  g_signal_connect_swapped(app, "startup", G_CALLBACK(onStartup), this);
  g_signal_connect_swapped(app, "activate", G_CALLBACK(onActivate), this);
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
  self.data = new UiProgramData(self.config);
  g_cthrd = new UiCoreThread(*self.data);
  self.window = new MainWindow(self.app, *g_cthrd);
  g_progdt = self.data;
  g_mwin = self.window;

  iptux_register_resource();
  GActionEntry app_entries[] =  {
      { "quit", G_ACTION_CALLBACK(onQuit), NULL, NULL, NULL, {0,0,0}},
      { "preferences", G_ACTION_CALLBACK(onPreferences), NULL, NULL, NULL, {0,0,0}},
      { "help.faq", G_ACTION_CALLBACK(HelpDialog::onFaq), NULL, NULL, NULL, {0,0,0}},
      { "tools.transmission", G_ACTION_CALLBACK(onToolsTransmission), NULL, NULL, NULL, {0,0,0}},
      { "tools.shared_management", G_ACTION_CALLBACK(onToolsSharedManagement), NULL, NULL, NULL, {0,0,0}},
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
  StatusIcon* sicon = new StatusIcon(self.config, *self.window);
  g_sndsys = new SoundSystem();

  self.window->SetStatusIcon(sicon);
  self.window->CreateWindow();
  try {
    g_cthrd->start();
  } catch (const BindFailedException& e) {
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
}
