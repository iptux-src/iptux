#include "config.h"
#include "WindowConfig.h"

using namespace std;

namespace iptux {

void WindowConfig::LoadFromConfig(shared_ptr<IptuxConfig> config) {
  int width = config->GetInt(prefix + "_width");
  if (width != 0) {
    this->width = width;
  }

  int height = config->GetInt(prefix + "_height");
  if (height != 0) {
    this->height = height;
  }
}

void WindowConfig::SaveToConfig(shared_ptr<IptuxConfig> config) {
  config->SetInt(prefix + "_width", width);
  config->SetInt(prefix + "_height", height);
  config->Save();
}

}  // namespace iptux
