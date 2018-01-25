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
#ifndef IPTUX_STATUSICON_H
#define IPTUX_STATUSICON_H

#include "iptux/IptuxConfig.h"
#include "iptux/MainWindow.h"

namespace iptux {

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
  GtkStatusIcon* statusicon;
  guint timerid;
  gboolean embedded;

 private:
  ProgramData& getProgramData() { return mwin.GetProgramData(); }
  static gboolean UpdateUI(StatusIcon* sicon);
  GtkWidget* CreatePopupMenu();
  //回调处理部分
 private:
  static void StatusIconActivate(StatusIcon* self);
  static void onPopupMenu(StatusIcon* self, guint button, guint time);
  static gboolean StatusIconQueryTooltip(GtkStatusIcon* statusicon, gint x,
                                         gint y, gboolean key,
                                         GtkTooltip* tooltip);
  static gboolean onActivate(StatusIcon* self);
};

}  // namespace iptux

#endif
