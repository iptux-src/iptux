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
#ifndef IPTUX_DIALOG_H
#define IPTUX_DIALOG_H

#include "iptux/mess.h"

namespace iptux {

bool pop_request_quit(GtkWindow* parent);
bool pop_request_shared_file(PalInfo *pal);
char *pop_obtain_shared_passwd(PalInfo *pal);
char *pop_password_settings(GtkWidget *parent);

/**
 * 弹出接收文件存放位置的对话框.
 * @param parent parent window
 * @return path const char*, if user does not accept, return nullptr
 */
const char *pop_save_path(GtkWidget *parent);

}  // namespace iptux

#endif
