//
// C++ Interface: face
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef FACE_H
#define FACE_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_GST
#include <gst/gst.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#undef _
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>

#define ATOM_OBJECT 0xFFFC
#define OCCUPY_OBJECT 0x0001
#define GINT(x) ((gint)(x))

#define GCONF_PATH "/apps/iptux"
#define IPTUX_PATH "/iptux"
#define ADS_PATH "/iptux/ads"
#define ICON_PATH "/iptux/icon"
#define PIC_PATH "/iptux/pic"
#define LOG_PATH "/iptux/log"
#define COMPLEX_PATH "/iptux/complex"

#ifndef _
#define _(string) gettext(string)
#endif

#endif
