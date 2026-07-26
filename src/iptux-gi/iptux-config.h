#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define IPTUX_TYPE_CONFIG (iptux_config_get_type())
G_DECLARE_FINAL_TYPE(IptuxConfig, iptux_config, IPTUX, CONFIG, GObject)


IptuxConfig* iptux_config_new(void);

G_END_DECLS