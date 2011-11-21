//
// C++ Interface: StatusIcon
//
// Description:
// 创建状态栏图标
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef STATUSICON_H
#define STATUSICON_H

#include "mess.h"

class StatusIcon {
public:
        StatusIcon();
        ~StatusIcon();

        void CreateStatusIcon();
	gboolean IsEmbedded();
        void AlterStatusIconMode();
private:
        GtkStatusIcon *statusicon;
        guint timerid;
	gboolean embedded;
private:
        static gboolean UpdateUI(StatusIcon *sicon);
        static GtkWidget *CreatePopupMenu(GtkStatusIcon *statusicon);
//回调处理部分
private:
        static void ShowTransWindow();
        static void StatusIconActivate();
        static void PopupWorkMenu(GtkStatusIcon *statusicon, guint button, guint time);
        static gboolean StatusIconQueryTooltip(GtkStatusIcon *statusicon, gint x, gint y,
                                                 gboolean key, GtkTooltip *tooltip);
};

#endif
