//
// C++ Implementation: support
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "support.h"
#include "Control.h"
#include "UdpData.h"
#include "SendFile.h"
#include "Transport.h"
#include "Log.h"
#include "baling.h"
#include "output.h"
#include "dialog.h"
#include "utils.h"

void iptux_init()
{
	extern Control ctr;
	extern UdpData udt;
	extern SendFile sfl;
	extern Transport trans;
	extern Log mylog;

	bind_iptux_port();
	init_iptux_environment();

	ctr.InitSelf();
	udt.InitSelf();
	sfl.InitSelf();
	trans.InitSelf();
	mylog.InitSelf();

	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, (sighandler_t) iptux_quit);
	signal(SIGINT, (sighandler_t) iptux_quit);
	signal(SIGQUIT, (sighandler_t) iptux_quit);
	signal(SIGTERM, (sighandler_t) iptux_quit);
}

void iptux_gui_quit()
{
	extern Transport trans;

	if (trans.TransportActive() && !pop_request_quit())
		return;
	gtk_main_quit();
	iptux_quit();
}

void iptux_quit()
{
	pmessage(_("The messenger is quit!\n"));
	exit(0);
}

void pixbuf_shrink_scale_1(GdkPixbuf ** pixbuf, int width, int height)
{
	gdouble scale_x, scale_y, scale;
	gint _width, _height;
	GdkPixbuf *tmp;

	width = (width != -1) ? width : G_MAXINT;
	height = (height != -1) ? height : G_MAXINT;
	_width = gdk_pixbuf_get_width(*pixbuf);
	_height = gdk_pixbuf_get_height(*pixbuf);
	if (_width > width || _height > height) {
		scale = ((scale_x = (gdouble) width / _width) <
			 (scale_y = (gdouble) height / _height))
			 ? scale_x : scale_y;
		_width = (gint) (_width * scale);
		_height = (gint) (_height * scale);
		tmp = *pixbuf;
		*pixbuf = gdk_pixbuf_scale_simple(tmp, _width, _height,
					    GDK_INTERP_BILINEAR);
		g_object_unref(tmp);
	}
}

GdkPixbuf *obtain_pixbuf_from_stock(const gchar * stock_id)
{
	GtkWidget *widget;
	GdkPixbuf *pixbuf;

	widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	pixbuf = gtk_widget_render_icon(widget, stock_id,
					GTK_ICON_SIZE_MENU, NULL);
	gtk_widget_destroy(widget);

	return pixbuf;
}

/**
 * cache iptux {ads, icon, pic}
 * config iptux {log, complex}
 */
void init_iptux_environment()
{
	const char *env;
	char path[MAX_PATHBUF];

	env = g_get_user_cache_dir();
	if (access(env, F_OK) != 0)
		Mkdir(env, 0777);
	snprintf(path, MAX_PATHBUF, "%s/iptux", env);
	if (access(path, F_OK) != 0)
		Mkdir(path, 0777);
	snprintf(path, MAX_PATHBUF, "%s/iptux/ads", env);
	if (access(path, F_OK) != 0)
		Mkdir(path, 0777);
	snprintf(path, MAX_PATHBUF, "%s/iptux/icon", env);
	if (access(path, F_OK) != 0)
		Mkdir(path, 0777);
	snprintf(path, MAX_PATHBUF, "%s/iptux/pic", env);
	if (access(path, F_OK) != 0)
		Mkdir(path, 0777);

	env = g_get_user_config_dir();
	if (access(env, F_OK) != 0)
		Mkdir(env, 0777);
	snprintf(path, MAX_PATHBUF, "%s/iptux", env);
	if (access(path, F_OK) != 0)
		Mkdir(path, 0777);
	snprintf(path, MAX_PATHBUF, "%s/iptux/log", env);
	if (access(path, F_OK) != 0)
		Mkdir(path, 0777);
	snprintf(path, MAX_PATHBUF, "%s/iptux/complex", env);
	if (access(path, F_OK) != 0)
		Mkdir(path, 0777);
}

void bind_iptux_port()
{
	extern struct interactive inter;
	int tcpsock, udpsock;
	SI addr;

	inter.tcpsock = tcpsock = Socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	socket_enable_reuse(tcpsock);
	inter.udpsock = udpsock = Socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	socket_enable_reuse(udpsock);
	socket_enable_broadcast(udpsock);

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPTUX_DEFAULT_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(tcpsock, (SA *) & addr, sizeof(addr)) == -1
		   || bind(udpsock, (SA *) & addr, sizeof(addr)) == -1) {
		close(tcpsock), close(udpsock);
		pop_error(_("act: bind the TCP/UDP port(2425) !\nerror: %s !"),
			  strerror(errno));
		exit(1);
	}
}

void socket_enable_broadcast(int sock)
{
	socklen_t len;
	int optval;

	optval = 1;
	len = sizeof(optval);
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, len);
}

void socket_enable_reuse(int sock)
{
	socklen_t len;
	int optval;

	optval = 1;
	len = sizeof(optval);
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, len);
}

GSList *get_sys_broadcast_addr(int sock)
{
	const uint8_t amount = 5;
	uint8_t count, sum;
	struct ifconf ifc;
	struct ifreq *ifr;
	GSList *list;
	SI *addr;

	list = g_slist_append(NULL,
			   GUINT_TO_POINTER(inet_addr("255.255.255.255")));
	ifc.ifc_len = amount * sizeof(struct ifreq);
	ifc.ifc_buf = (caddr_t) Malloc(ifc.ifc_len);
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
		free(ifc.ifc_buf);
		return list;
	}
	sum = ifc.ifc_len / sizeof(struct ifreq);
	count = 0;
	while (count < sum) {
		ifr = ifc.ifc_req + count;
		count++;

		if (ioctl(sock, SIOCGIFFLAGS, ifr) == -1
			  || !(ifr->ifr_flags & IFF_BROADCAST)
			  || ioctl(sock, SIOCGIFBRDADDR, ifr) == -1)
			continue;
		addr = (SI *) & ifr->ifr_broadaddr;
		list = g_slist_append(list,
			     GUINT_TO_POINTER(addr->sin_addr.s_addr));
	}
	free(ifc.ifc_buf);

	return list;
}

GSList *get_sys_host_addr(int sock)
{
	const uint8_t amount = 5;
	uint8_t count, sum;
	struct ifconf ifc;
	struct ifreq *ifr;
	GSList *list;
	SI *addr;

	list = NULL;
	ifc.ifc_len = amount * sizeof(struct ifreq);
	ifc.ifc_buf = (caddr_t) Malloc(ifc.ifc_len);
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
		free(ifc.ifc_buf);
		return list;
	}
	sum = ifc.ifc_len / sizeof(struct ifreq);
	count = 0;
	while (count < sum) {
		ifr = ifc.ifc_req + count;
		count++;

		if (strncasecmp(ifr->ifr_name, "lo", 2) == 0
				  || ioctl(sock, SIOCGIFFLAGS, ifr) == -1
				  || !(ifr->ifr_flags & IFF_UP)
				  || ioctl(sock, SIOCGIFADDR, ifr) == -1)
			continue;
		addr = (SI *) & ifr->ifr_broadaddr;
		list = g_slist_append(list,
			     GUINT_TO_POINTER(addr->sin_addr.s_addr));
	}
	free(ifc.ifc_buf);

	return list;
}

char *get_sys_host_addr_string(int sock)
{
	char ipstr[INET_ADDRSTRLEN], *buf, *ptr;
	GSList *list, *tmp;
	uint16_t len;

	if (!(tmp = list = get_sys_host_addr(sock)))
		return NULL;
	len = g_slist_length(list) * INET_ADDRSTRLEN;
	ptr = buf = (char *)Malloc(len);
	while (tmp) {
		inet_ntop(AF_INET, &tmp->data, ipstr, INET_ADDRSTRLEN);
		snprintf(ptr, len, "%s\n", ipstr);
		ptr += strlen(ptr), len -= INET_ADDRSTRLEN;
		tmp = tmp->next;
	}
	*(ptr - 1) = '\0';

	g_slist_free(list);
	return buf;
}
