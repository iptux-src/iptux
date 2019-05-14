//
// C++ Interface: DialogGroup
//
// Description:
// 与多人对话
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_DIALOGGROUP_H
#define IPTUX_DIALOGGROUP_H

#include "iptux/DialogBase.h"
#include "iptux/IptuxConfig.h"
#include "iptux/UiProgramData.h"
#include "iptux/Models.h"
#include "iptux/MainWindow.h"

namespace iptux {

class DialogGroup : public DialogBase {
 public:
  DialogGroup(MainWindow* mainWindow, GroupInfo *grp, std::shared_ptr<UiProgramData> progdt);
  virtual ~DialogGroup();

  static void GroupDialogEntry(MainWindow* mainWindow, GroupInfo *grpinf,
                               std::shared_ptr<UiProgramData> progdt);

  virtual void UpdatePalData(PalInfo *pal);
  virtual void InsertPalData(PalInfo *pal);
  virtual void DelPalData(PalInfo *pal);
  virtual void ClearAllPalData();
  virtual GSList *GetSelPal();

 private:
  MainWindow* mainWindow;
  IptuxConfig &config;
  GtkWidget* mainPaned;
  GtkWidget* memberEnclosurePaned;
  GtkWidget* historyInputPaned;

 private:
  virtual void InitSublayerSpecify();
  void ReadUILayout();
  void SaveUILayout();

  GtkWindow *CreateMainWindow();
  GtkWidget *CreateAllArea();

  GtkWidget *CreateMenuBar();
  GtkWidget *CreateMemberArea();
  GtkWidget *CreateInputArea();
  GtkWidget *CreateToolMenu();

  GtkTreeModel *CreateMemberModel();
  void FillMemberModel(GtkTreeModel *model);
  GtkWidget *CreateMemberTree(GtkTreeModel *model);

  bool SendTextMsg();
  void BroadcastEnclosureMsg(GSList *list);
  void BroadcastTextMsg(const gchar *msg);

  static GtkWidget *CreatePopupMenu(GtkTreeModel *model);
  //回调处理部分
 private:
  static void onUIChanged(DialogGroup& self);
  static gint MemberTreeCompareByNameFunc(GtkTreeModel *model, GtkTreeIter *a,
                                          GtkTreeIter *b);
  static gint MemberTreeCompareByIPFunc(GtkTreeModel *model, GtkTreeIter *a,
                                        GtkTreeIter *b);
  static void SetMemberTreeSortFunc(GtkWidget *menuitem, GtkTreeModel *model);
  static void SetMemberTreeSortType(GtkWidget *menuitem, GtkTreeModel *model);
  static void DragDataReceived(DialogGroup *dlggrp, GdkDragContext *context,
                               gint x, gint y, GtkSelectionData *data,
                               guint info, guint time);
  static gboolean PopupPickMenu(GtkWidget *treeview, GdkEventButton *event);
  static void MembertreeItemActivated(GtkWidget *treeview, GtkTreePath *path,
                                      GtkTreeViewColumn *column,
                                      DialogGroup *self);
  static void SendMessage(DialogGroup *dlggrp);
  static void onActive(DialogGroup& self);
};

}  // namespace iptux

#endif
