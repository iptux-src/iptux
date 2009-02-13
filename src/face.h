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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#undef _
#define _(string) gettext(string)

#define GCONF_PATH "/apps/iptux"
#define ATOM_OBJECT 0xFFFC
#define OCCUPY_OBJECT 0x0001
#define GINT(x) ((gint)(x))

#endif
