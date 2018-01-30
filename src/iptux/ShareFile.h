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

namespace iptux {

typedef GtkDialog ShareFile;
ShareFile* share_file_new(GtkWindow* parent);
void share_file_run(ShareFile* dialog);

}  // namespace iptux

#endif
