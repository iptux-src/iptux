#include "WindowConfig.h"

namespace iptux {

void WindowConfig::LoadFromConfig(IptuxConfig& config) {
  int width = config.GetInt(prefix + "_width");
  if (width != 0) {
    this->width = width;
  }

  int height = config.GetInt(prefix + "_height");
  if (height != 0) {
    this->height = height;
  }
}

void WindowConfig::SaveToConfig(IptuxConfig& config) {
  config.SetInt(prefix + "_width", width);
  config.SetInt(prefix + "_height", height);
  config.Save();
}

}  // namespace iptux
