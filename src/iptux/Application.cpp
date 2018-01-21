#include "Application.h"
#include "iptux/ProgramData.h"
#include "iptux/global.h"
#include "iptux/ipmsg.h"
#include "iptux/support.h"
#include "iptux/StatusIcon.h"
#include "HelpDialog.h"

static const char* menuUi = "<?xml version=\"1.0\"?>\n"
    "<interface>\n"
    "  <menu id=\"appmenu\">\n"
    "    <section>\n"
    "      <item>\n"
    "        <attribute name=\"label\" translatable=\"yes\">_Quit</attribute>\n"
    "        <attribute name=\"action\">app.quit</attribute>\n"
    "      </item>\n"
    "    </section>\n"
    "  </menu>"
    "  <menu id='menubar'>"
    "  <submenu>"
    "    <attribute name='label' translatable='yes'>_Help</attribute>"
    "    <section>\n"
    "      <item>\n"
    "        <attribute name=\"label\" translatable=\"yes\">_About</attribute>\n"
    "        <attribute name=\"action\">app.about</attribute>\n"
    "      </item>\n"
    "    </section>\n"
    "  </submenu>"
    "  </menu>\n"
    "</interface>";


typedef void (* GActionCallback) (GSimpleAction *action,
                   GVariant      *parameter,
                   gpointer       user_data) ;
#define	G_ACTION_CALLBACK(f)			 ((GActionCallback) (f))

namespace iptux {

Application::Application(IptuxConfig& config)
: config(config) {
  app = gtk_application_new ("io.github.iptux-src.iptux", G_APPLICATION_FLAGS_NONE);
  g_signal_connect_swapped(app, "startup", G_CALLBACK(onStartup), this);
  g_signal_connect_swapped(app, "activate", G_CALLBACK(onActivate), this);
}

Application::~Application() {
  g_object_unref(app);
}

int Application::run(int argc, char** argv) {
  return g_application_run (G_APPLICATION (app), argc, argv);
}

void Application::onStartup(Application& self) {
  GActionEntry app_entries[] =  {
      { "quit", G_ACTION_CALLBACK(Application::onQuit), NULL, NULL, NULL },
      { "about", G_ACTION_CALLBACK(HelpDialog::AboutEntry), NULL, NULL, NULL }
  };
  g_action_map_add_action_entries (G_ACTION_MAP (self.app),
                                   app_entries, G_N_ELEMENTS (app_entries),
                                   &self);
  auto builder = gtk_builder_new_from_string (
      menuUi, -1);
  auto app_menu = G_MENU_MODEL (gtk_builder_get_object (builder, "appmenu"));
  gtk_application_set_app_menu (GTK_APPLICATION (self.app), app_menu);
  auto menubar = G_MENU_MODEL (gtk_builder_get_object (builder, "menubar"));
  gtk_application_set_menubar(GTK_APPLICATION(self.app), menubar);
  g_object_unref (builder);
}

void Application::onActivate(Application& self) {
  g_progdt = new ProgramData(self.config);
  g_mwin = new MainWindow(self.app, self.config, *g_progdt);
  g_cthrd = new CoreThread(self.config);
  StatusIcon* sicon = new StatusIcon(self.config, *g_mwin);
  g_sndsys = new SoundSystem();
  g_lgsys = new LogSystem();

  g_mwin->SetStatusIcon(sicon);

  int port = self.config.GetInt("port", IPTUX_DEFAULT_PORT);
  iptux_init(port);
  sicon->CreateStatusIcon();
  g_mwin->CreateWindow();
  g_cthrd->CoreThreadEntry();
}

void Application::onQuit (GSimpleAction *action,
             GVariant      *parameter,
             Application& self) {
  g_application_quit(G_APPLICATION (self.app));
}

void Application::onAbout (GSimpleAction *action,
                          GVariant      *parameter,
                          Application& self) {
  g_application_quit(G_APPLICATION (self.app));
}


}