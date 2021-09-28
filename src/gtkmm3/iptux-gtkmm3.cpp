#include "MainWindow.h"

int main(int argc, char* argv[]) {
  auto app = Gtk::Application::create("org.gtkmm.examples.base");

  MainWindow main_window;

  app->run(main_window);
  return 0;
}
