#include "Iptux.h"
#include "MainWindow.h"

Iptux::Iptux() : Gtk::Application("io.github.iptux_src.iptux_gtkmm4") {}

Glib::RefPtr<Iptux> Iptux::create() {
  return Glib::make_refptr_for_instance<Iptux>(new Iptux());
}

void Iptux::on_activate() {
  auto window = new MainWindow();
  window->present();
  add_window(*window);
}
