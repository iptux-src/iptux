#pragma once

#include <glib-object.h>
#include "iptux-core/IptuxConfig.h"

struct _IptuxConfig {
  GObject parent_instance;
  iptux::IptuxConfig::Ptr config;
};