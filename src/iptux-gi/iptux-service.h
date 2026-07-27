#pragma once

#include "iptux-config.h"
#include "iptux-pal.h"
#include <glib-object.h>

G_BEGIN_DECLS

#define IPTUX_TYPE_SERVICE (iptux_service_get_type())
G_DECLARE_FINAL_TYPE(IptuxService, iptux_service, IPTUX, SERVICE, GObject)

IptuxService* iptux_service_new(IptuxConfig* config);

bool iptux_service_start(IptuxService* self);
bool iptux_service_stop(IptuxService* self);

/**
 * iptux_service_get_pals:
 * @self: An #IptuxService instance.
 *
 * Gets the list of pals.
 *
 * Returns: (element-type IptuxPal) (transfer container): A #GArray of pals.
 */
GArray* iptux_service_get_pals(IptuxService* self);

gboolean iptux_service_send_message(IptuxService* self,
                                    IptuxPal* pal,
                                    const gchar* message,
                                    GError** error);

G_END_DECLS
