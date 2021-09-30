#include "config.h"
#include "Iptux.h"
#include "IptuxResource.h"
#include "MainWindow.h"
#include <iostream>

using namespace std;

namespace iptux {

Iptux::Iptux(shared_ptr<IptuxConfig> config)
    : Gtk::Application(config->GetString("debug_application_id",
                                         "io.github.iptux_src.iptux_gtkmm4")),
      config(config) {}

Glib::RefPtr<Iptux> Iptux::create(shared_ptr<IptuxConfig> config) {
  return Glib::make_refptr_for_instance<Iptux>(new Iptux(config));
}

void Iptux::on_startup() {
  Gtk::Application::on_startup();
  iptux_register_resource();
  cout << "Iptux::on_startup" << endl;
  this->menuBuilder =
      Gtk::Builder::create_from_resource(IPTUX_RESOURCE "gtk/menus.ui");
  this->set_menubar(menuBuilder->get_object<Gio::MenuModel>("menubar"));
  this->add_action("about", sigc::mem_fun(*this, &Iptux::on_about));
}

void Iptux::on_activate() {
  cout << "Iptux::on_activate" << endl;
  data = make_shared<ProgramData>(config);
  coreThread = make_shared<CoreThread>(data);
  coreThread->start();

  auto window = new MainWindow();
  window->present();
  add_window(*window);
}

void Iptux::on_about() {
  cout << "Iptux::on_about" << endl;
}

}  // namespace iptux
