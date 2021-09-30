#include "MainWindow.h"

namespace iptux {

MainWindow::MainWindow() {
  set_title("Iptux");
  set_default_size(200, 200);

  auto hbox = Gtk::manage(new Gtk::Box());
  set_child(*hbox);

  auto vbox = Gtk::manage(new Gtk::Box(Gtk::Orientation::VERTICAL));
  hbox->append(*vbox);

  auto label = Gtk::manage(new Gtk::Label("some buttons in 64x64px"));
  vbox->append(*label);
}

}  // namespace iptux
