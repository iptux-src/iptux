#include <gtkmm.h>

class MainWindow : public Gtk::Window {
 public:
  MainWindow();
};

MainWindow::MainWindow() {
  set_title("Basic application");
  set_default_size(200, 200);
}
