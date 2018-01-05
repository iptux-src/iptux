//
// C++ Interface: dialog
//
// Description:
// 常见对话框构建
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DIALOG_H
#define DIALOG_H

#include "mess.h"

bool pop_request_quit();
bool pop_request_shared_file(PalInfo *pal);
char *pop_obtain_shared_passwd(PalInfo *pal);
char *pop_password_settings(GtkWidget *parent);
char *pop_save_path(GtkWidget *parent);
#endif
