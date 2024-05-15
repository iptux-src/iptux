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

#include <string>
#include <vector>

namespace iptux {

void socket_enable_broadcast(int sock);
void socket_enable_reuse(int sock);
std::vector<std::string> get_sys_broadcast_addr(int sock);

}  // namespace iptux

#endif
