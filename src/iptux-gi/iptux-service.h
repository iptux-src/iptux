#pragma once

#include "iptux-config.h"
#include <glib-object.h>

G_BEGIN_DECLS

#define IPTUX_TYPE_SERVICE (iptux_service_get_type())
G_DECLARE_FINAL_TYPE(IptuxService, iptux_service, IPTUX, SERVICE, GObject)

IptuxService* iptux_service_new(IptuxConfig* config);

bool iptux_service_start(IptuxService* self);
bool iptux_service_stop(IptuxService* self);

G_END_DECLS