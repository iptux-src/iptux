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
#include "MainWindow.h"

class StatusIcon {
public:
  StatusIcon(IptuxConfig& config, MainWindow& mwin);
  ~StatusIcon();

  void CreateStatusIcon();
	gboolean IsEmbedded();
  void AlterStatusIconMode();
  gboolean AlterInterfaceMode();
private:
  IptuxConfig& config;
  MainWindow& mwin;
  GtkStatusIcon *statusicon;
  guint timerid;
	gboolean embedded;

private:
  ProgramData& getProgramData() {
    return mwin.GetProgramData();
  }
  static gboolean UpdateUI(StatusIcon *sicon);
  GtkWidget* CreatePopupMenu();
//回调处理部分
private:
  static void ShowTransWindow(StatusIcon* self);
  static void StatusIconActivate(StatusIcon* self);
  static void onPopupMenu(GtkStatusIcon *statusicon, guint button, guint time, StatusIcon* self);
  static gboolean StatusIconQueryTooltip(GtkStatusIcon *statusicon, gint x, gint y,
                                            gboolean key, GtkTooltip *tooltip);
  static gboolean onActivate(StatusIcon* self);
};

#endif
