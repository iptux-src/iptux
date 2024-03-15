//
// C++ Interface: ShareFile
//
// Description:
// 添加或删除共享文件,即管理共享文件
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_SHAREFILE_H
#define IPTUX_SHAREFILE_H

#include <gtk/gtk.h>

#include "iptux/Application.h"

namespace iptux {

using ShareFile = GtkDialog;
ShareFile* shareFileNew(Application* app);
void shareFileRun(ShareFile* dialog, GtkWindow* parent);

}  // namespace iptux

#endif
