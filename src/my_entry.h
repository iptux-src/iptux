//
// C++ Interface: my_entry
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MY_ENTRY_H
#define MY_ENTRY_H

#include "face.h"

class my_entry {
 public:
	my_entry();
	~my_entry();

	static GtkWidget *create_entry(const char *text, const char *tip,
				       bool digital = false);
 private:
	static gboolean QueryTooltip(GtkWidget * widget, gint x, gint y,
				     gboolean key, GtkTooltip * tooltip,
				     gpointer data);
	static void InsertText(GtkEditable * editable, gchar * text,
			       gint length, gint * position, gpointer data);
};

#endif
