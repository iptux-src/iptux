#ifndef IPTUX_WINDOW_CONFIG_H
#define IPTUX_WINDOW_CONFIG_H

#include <string>

#include "iptux/IptuxConfig.h"

namespace iptux {

class WindowConfig {
 public:
  WindowConfig(int defaultWidth, int defaultHeight, std::string prefix) {
    this->width = defaultWidth;
    this->height = defaultHeight;
    this->prefix = prefix;
  }

  virtual ~WindowConfig() {}

  int GetWidth() const { return width; }

  WindowConfig& SetWidth(int width) {
    this->width = width;
    return *this;
  }

  int GetHeight() const { return height; }

  WindowConfig& SetHeight(int height) {
    this->height = height;
    return *this;
  }

  void LoadFromConfig(IptuxConfig& config);
  void SaveToConfig(IptuxConfig& config);

 private:
  int width;
  int height;
  std::string prefix;
};

}  // namespace iptux

#endif