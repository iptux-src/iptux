#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define IPTUX_TYPE_SERVICE (iptux_service_get_type())
G_DECLARE_FINAL_TYPE(IptuxService, iptux_service, IPTUX, SERVICE, GObject)


IptuxService* iptux_service_new(void);

G_END_DECLS