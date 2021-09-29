#ifndef IPTUX_H
#define IPTUX_H

#include "iptux-core/CoreThread.h"
#include <gtkmm.h>

namespace iptux {

class Iptux : public Gtk::Application {
 protected:
  explicit Iptux(std::shared_ptr<IptuxConfig> config);

 public:
  static Glib::RefPtr<Iptux> create(std::shared_ptr<IptuxConfig> config);

 protected:
  void on_activate() override;

 private:
  std::shared_ptr<IptuxConfig> config;
  std::shared_ptr<ProgramData> data;
  std::shared_ptr<CoreThread> coreThread;
};

}  // namespace iptux

#endif
