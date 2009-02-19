//
// C++ Interface: dialog
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DIALOG_H
#define DIALOG_H

#include "udt.h"

bool pop_request_quit();
bool pop_request_shared(gpointer data);	//Pal
char *pop_obtain_passwd();
char *pop_passwd_setting(GtkWidget *parent);

#endif
