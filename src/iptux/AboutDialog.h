//
//
// Description:
// 创建帮助相关对话框
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_ABOUT_DIALOG_H
#define IPTUX_ABOUT_DIALOG_H

#include <gtk/gtk.h>

namespace iptux {

typedef GtkAboutDialog AboutDialog;
AboutDialog* aboutDialogNew();
void aboutDialogRun(AboutDialog* aboutDialog, GtkWindow* parent);
void aboutDialogEntry(GtkWindow* parent);

}  // namespace iptux

#endif
