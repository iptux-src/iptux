#ifndef IPTUX_H
#define IPTUX_H

#include <gtkmm.h>

class Iptux : public Gtk::Application {
 protected:
  Iptux();

 public:
  static Glib::RefPtr<Iptux> create();

 protected:
  void on_activate() override;
};

#endif
