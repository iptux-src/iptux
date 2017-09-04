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
#include "IptuxConfig.h"

class StatusIcon {
public:
        StatusIcon(IptuxConfig& config);
        ~StatusIcon();

        void CreateStatusIcon();
	gboolean IsEmbedded();
        void AlterStatusIconMode();
private:
    IptuxConfig& config;
        GtkStatusIcon *statusicon;
        guint timerid;
	gboolean embedded;
private:
        static gboolean UpdateUI(StatusIcon *sicon);
        static GtkWidget *CreatePopupMenu(GtkStatusIcon *statusicon);
//回调处理部分
private:
        static void ShowTransWindow();
        static void StatusIconActivate(StatusIcon* self);
        static void PopupWorkMenu(GtkStatusIcon *statusicon, guint button, guint time);
        static gboolean StatusIconQueryTooltip(GtkStatusIcon *statusicon, gint x, gint y,
                                                 gboolean key, GtkTooltip *tooltip);
};

#endif
