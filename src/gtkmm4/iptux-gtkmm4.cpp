#include "Iptux.h"

int main(int argc, char* argv[]) {
  auto app = Iptux::create();
  return app->run(argc, argv);
}
