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
#ifndef SUPPORT_H
#define SUPPORT_H

#include "face.h"

void iptux_init();		//初始化程序数据
void iptux_gui_quit();		//图形用户界面退出
void iptux_quit();		//底层退出

void update_widget_bg(GtkWidget * widget, const gchar * file);
void pixbuf_shrink_scale_1(GdkPixbuf ** pixbuf, int width, int height);
GdkPixbuf *obtain_pixbuf_from_stock(const gchar * stock_id);

void init_iptux_environment();
void bind_iptux_port();

void socket_enable_broadcast(int sock);
void socket_enable_reuse(int sock);
GSList *get_sys_broadcast_addr(int sock);
GSList *get_sys_host_addr(int sock);
char *get_sys_host_addr_string(int sock);

#endif
