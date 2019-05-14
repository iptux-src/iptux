#ifndef IPTUX_WINDOW_CONFIG_H
#define IPTUX_WINDOW_CONFIG_H

#include <string>
#include <memory>

#include "iptux/IptuxConfig.h"

namespace iptux {

class WindowConfig {
 public:
  WindowConfig(int defaultWidth, int defaultHeight, const std::string& prefix)
  : width(defaultWidth),
    height(defaultHeight),
    prefix(prefix)
  {
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

  void LoadFromConfig(std::shared_ptr<IptuxConfig> config);
  void SaveToConfig(std::shared_ptr<IptuxConfig> config);

 private:
  int width;
  int height;
  std::string prefix;
};

}  // namespace iptux

#endif
