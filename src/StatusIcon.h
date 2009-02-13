//
// C++ Interface: StatusIcon
//
// Description:创建状态栏图标
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef STATUSICON_H
#define STATUSICON_H

#include "face.h"

class StatusIcon {
 public:
	StatusIcon();
	~StatusIcon();

	void CreateStatusIcon();
 private:
	 GtkStatusIcon * status_icon;
 public:
	static void UpdateTips();
 private:
	 GtkWidget * CreatePopupMenu();
//回调处理部分
 public:
	static void SwitchWindowMode();
 private:
	static void StatusIconActivate();
	static void PopupWorkMenu(GtkStatusIcon * status_icon, guint button,
				  guint activate_time, gpointer data);	//StatusIcon
};

#endif
