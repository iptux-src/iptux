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

#include "iptux-core/IptuxConfig.h"
#include "iptux-core/Models.h"
#include "iptux/Application.h"
#include "iptux/DialogBase.h"

namespace iptux {

class DialogGroup : public DialogBase {
 public:
  DialogGroup(Application* app, GroupInfo* grp);
  virtual ~DialogGroup();

  static DialogGroup* GroupDialogEntry(Application* app, GroupInfo* grpinf);
  void UpdatePalData(PalInfo* pal) override;
  void InsertPalData(PalInfo* pal) override;
  void DelPalData(PalInfo* pal) override;
  void ClearAllPalData() override;
  GSList* GetSelPal() override;
  GtkWindow* getWindow() override { return GTK_WINDOW(window); }

 private:
  GtkApplicationWindow* window;
  std::shared_ptr<IptuxConfig> config;
  GtkWidget* mainPaned;
  GtkWidget* memberEnclosurePaned;
  GtkWidget* historyInputPaned;

 private:
  void InitSublayerSpecify();
  void ReadUILayout();
  void SaveUILayout();

  GtkWindow* CreateMainWindow();
  void CreateTitle();
  GtkWidget* CreateAllArea();

  GtkWidget* CreateMemberArea();
  GtkWidget* CreateInputArea();

  GtkTreeModel* CreateMemberModel();
  void FillMemberModel(GtkTreeModel* model);
  GtkWidget* CreateMemberTree(GtkTreeModel* model);

  bool SendTextMsg() override;
  void BroadcastEnclosureMsg(const std::vector<FileInfo*>& files) override;
  void broadcastTextMsg(std::shared_ptr<MsgPara> para);
  std::string GetTitle();

  static GtkWidget* CreatePopupMenu(GtkTreeModel* model);
  // 回调处理部分
 private:
  static void onUIChanged(DialogGroup& self);
  static gint MemberTreeCompareByNameFunc(GtkTreeModel* model,
                                          GtkTreeIter* a,
                                          GtkTreeIter* b);
  static gint MemberTreeCompareByIPFunc(GtkTreeModel* model,
                                        GtkTreeIter* a,
                                        GtkTreeIter* b);
  static void SetMemberTreeSortFunc(GtkWidget* menuitem, GtkTreeModel* model);
  static void SetMemberTreeSortType(GtkWidget* menuitem, GtkTreeModel* model);
  static void DragDataReceived(DialogGroup* dlggrp,
                               GdkDragContext* context,
                               gint x,
                               gint y,
                               GtkSelectionData* data,
                               guint info,
                               guint time);
  static gboolean PopupPickMenu(GtkWidget* treeview, GdkEventButton* event);
  static void MembertreeItemActivated(GtkWidget* treeview,
                                      GtkTreePath* path,
                                      GtkTreeViewColumn* column,
                                      DialogGroup* self);
  static void SendMessage(DialogGroup* dlggrp);
  static void onClearChatHistory(void*, void*, DialogGroup& self) {
    self.ClearHistoryTextView();
  }
  static void onAttachFile(void*, void*, DialogGroup& self) {
    DialogBase::AttachRegular(&self);
  }
  static void onAttachFolder(void*, void*, DialogGroup& self) {
    DialogBase::AttachFolder(&self);
  }
  static void onSortType(GSimpleAction* action,
                         GVariant* value,
                         DialogGroup& self);
  static void onSortBy(GSimpleAction* action,
                       GVariant* value,
                       DialogGroup& self);
  static void onSendMessage(void*, void*, DialogGroup& self) {
    DialogBase::SendMessage(&self);
  }
};

}  // namespace iptux

#endif
