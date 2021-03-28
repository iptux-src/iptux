//
// C++ Interface: support
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_SUPPORT_H
#define IPTUX_SUPPORT_H

#include <stdexcept>

#include <glib.h>

#include "iptux-core/Models.h"

namespace iptux {

void socket_enable_broadcast(int sock);
void socket_enable_reuse(int sock);
GSList* get_sys_broadcast_addr(int sock);
GSList* get_sys_host_addr(int sock);
char* get_sys_host_addr_string(int sock);

}  // namespace iptux

#endif
