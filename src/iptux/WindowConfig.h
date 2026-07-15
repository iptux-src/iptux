#ifndef IPTUX_WINDOW_CONFIG_H
#define IPTUX_WINDOW_CONFIG_H

#include "iptux-core/IptuxConfig.h"
#include <glib.h>
#include <gtk/gtk.h>
#include <string>

namespace iptux {

class WindowConfig {
 public:
  WindowConfig(IptuxConfig::Ptr config,
               int defaultWidth,
               int defaultHeight,
               const std::string& prefix)
      : config(config),
        width(defaultWidth),
        height(defaultHeight),
        prefix(prefix) {
    this->LoadFromConfig(this->config);
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

  void LoadFromConfig(IptuxConfig::Ptr config);
  void SaveToConfig(IptuxConfig::Ptr config);
  void Save();

 public:
  static gboolean on_configure_event(GtkWidget* window,
                                     GdkEventConfigure* event,
                                     gpointer user_data);

 private:
  IptuxConfig::Ptr config;
  int width;
  int height;
  std::string prefix;
};

}  // namespace iptux

#endif
