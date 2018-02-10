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

#include "iptux/mess.h"

namespace iptux {

class BindFailedException: public std::runtime_error {
 public:
  BindFailedException(int ec, const std::string& reason)
  : std::runtime_error(reason)
  {
  }

};

/**
 *
 * @param port port number for binding
 * @throw BindFailedException if bind failed
 */
void iptux_init(int port);
void iptux_open_url(const char *url);
void init_iptux_environment();

void pixbuf_shrink_scale_1(GdkPixbuf **pixbuf, int width, int height);

void widget_enable_dnd_uri(GtkWidget *widget);
GSList *selection_data_get_path(GtkSelectionData *data);

char *ipv4_get_lan_name(in_addr_t ipv4);

void socket_enable_broadcast(int sock);
void socket_enable_reuse(int sock);
GSList *get_sys_broadcast_addr(int sock);
GSList *get_sys_host_addr(int sock);
char *get_sys_host_addr_string(int sock);

}  // namespace iptux

#endif
