//
// C++ Interface: RevisePal
//
// Description:手动更改好友数据
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_REVISEPAL_H
#define IPTUX_REVISEPAL_H

#include <gtk/gtk.h>

#include "iptux-core/Models.h"
#include "iptux/MainWindow.h"

namespace iptux {

class RevisePal {
 public:
  RevisePal(MainWindow* mainWin, PalInfo* pl);
  ~RevisePal();

  static void ReviseEntryDo(MainWindow* mainWin, PalInfo* pal, bool run);
  static void ReviseEntry(MainWindow* mainWin, PalInfo* pal) {
    ReviseEntryDo(mainWin, pal, true);
  }

 private:
  void InitSublayer();
  void ClearSublayer();

  GtkWidget* CreateMainDialog();
  GtkWidget* CreateAllArea();
  void SetAllValue();
  void ApplyReviseData();

  GtkTreeModel* CreateIconModel();
  void FillIconModel(GtkTreeModel* model);
  GtkWidget* CreateIconTree(GtkTreeModel* model);

  GData* widset;
  GData* mdlset;
  MainWindow* mainWin;
  PalInfo* pal;

 private:
  static gint IconfileGetItemPos(GtkTreeModel* model, const char* pathname);
  //回调处理部分
 private:
  static void AddNewIcon(GtkWidget* button, GData** widset);
};

}  // namespace iptux

#endif
