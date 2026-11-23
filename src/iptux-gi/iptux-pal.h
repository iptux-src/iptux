#pragma once

#include "iptux-config.h"
#include <glib-object.h>

G_BEGIN_DECLS

#define IPTUX_TYPE_PAL (iptux_pal_get_type())
G_DECLARE_FINAL_TYPE(IptuxPal, iptux_pal, IPTUX, PAL, GObject)

/**
 * iptux_pal_to_string:
 * @self: an #IptuxPal
 *
 * Returns: (transfer full): a human readable description of the pal
 */
char* iptux_pal_to_string(IptuxPal* self);

/**
 * iptux_pal_get_ipv4:
 * @self: an #IptuxPal
 *
 * Returns: (transfer full): the IPv4 address of the pal
 */
char* iptux_pal_get_ipv4(IptuxPal* self);

/**
 * iptux_pal_get_port:
 * @self: an #IptuxPal
 *
 * Returns: the port of the pal
 */
guint16 iptux_pal_get_port(IptuxPal* self);

G_END_DECLS
