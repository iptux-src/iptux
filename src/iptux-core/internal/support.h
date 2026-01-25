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

#include <gio/gio.h>
#include <string>
#include <vector>

namespace iptux {

void socket_enable_reuse(int sock);
std::vector<std::string> get_sys_broadcast_addr(GSocket* sock);

}  // namespace iptux

#endif
