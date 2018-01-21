#include "Application.h"
#include "iptux/ProgramData.h"
#include "iptux/global.h"
#include "iptux/ipmsg.h"
#include "iptux/support.h"
#include "iptux/StatusIcon.h"

namespace iptux {

Application::Application(IptuxConfig& config)
: config(config) {
  app = gtk_application_new ("io.github.iptux-src.iptux", G_APPLICATION_FLAGS_NONE);
  g_signal_connect_swapped(app, "activate", G_CALLBACK(onActivate), this);
}

Application::~Application() {
  g_object_unref(app);
}

int Application::run(int argc, char** argv) {
  return g_application_run (G_APPLICATION (app), argc, argv);
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



}