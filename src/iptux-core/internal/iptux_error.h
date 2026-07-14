#pragma once

#include <glib.h>

G_BEGIN_DECLS

#define IPTUX_CORE_ERROR g_quark_from_static_string("iptux-core-error")

enum IptuxCoreError {
  IPTUX_CORE_ERROR_UNKNOWN,
  IPTUX_CORE_ERROR_INVALID_MSG,
  IPTUX_CORE_ERROR_SOCKET,
  IPTUX_CORE_ERROR_FILE,
  IPTUX_CORE_ERROR_UNKNOWN_PEER,
};

G_END_DECLS


