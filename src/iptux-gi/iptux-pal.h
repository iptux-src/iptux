#pragma once

#include "iptux-config.h"
#include <glib-object.h>

G_BEGIN_DECLS

#define IPTUX_TYPE_PAL (iptux_pal_get_type())
G_DECLARE_FINAL_TYPE(IptuxPal, iptux_pal, IPTUX, PAL, GObject)

char* iptux_pal_to_string(IptuxPal* self);

G_END_DECLS
