#pragma once

#include "iptux-core/IptuxConfig.h"
#include "iptux-core/Models.h"
#include <glib-object.h>

struct _IptuxConfig {
  GObject parent_instance;
  iptux::IptuxConfig::Ptr config;
};

struct _IptuxPal {
  GObject parent_instance;
  iptux::PalInfo::Ptr pal_info;
};
