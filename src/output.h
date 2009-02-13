//
// C++ Interface: output
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef OUTPUT_H
#define OUTPUT_H

#include "udt.h"

void pwarning(enum STATE_TYPE state, const char *format, ...);
void pmessage(const char *format, ...);
void ptrace(const char *format, ...);

void pop_info(GtkWidget * parent, GtkWidget * fw, const gchar * format, ...);
void pop_warning(GtkWidget * parent, GtkWidget * fw, const gchar * format, ...);
void pop_error(const gchar * format, ...);

#endif
