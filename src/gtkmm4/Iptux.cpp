#include "Iptux.h"

#include "MainWindow.h"

using namespace std;

namespace iptux {

Iptux::Iptux(shared_ptr<IptuxConfig> config)
    : Gtk::Application(config->GetString("debug_application_id",
                                         "io.github.iptux_src.iptux_gtkmm4")),
      config(config) {}

Glib::RefPtr<Iptux> Iptux::create(shared_ptr<IptuxConfig> config) {
  return Glib::make_refptr_for_instance<Iptux>(new Iptux(config));
}

void Iptux::on_activate() {
  data = make_shared<ProgramData>(config);
  coreThread = make_shared<CoreThread>(data);
  coreThread->start();

  auto window = new MainWindow();
  window->present();
  add_window(*window);
}

}  // namespace iptux
