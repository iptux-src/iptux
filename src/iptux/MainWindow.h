//
// C++ Interface: MainWindow
//
// Description:
// 创建程序主窗口
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_MAINWINDOW_H
#define IPTUX_MAINWINDOW_H

#include "iptux/IptuxConfig.h"
#include "iptux/ProgramData.h"
#include "iptux/WindowConfig.h"
#include "iptux/mess.h"

namespace iptux {

class StatusIcon;

enum class ActiveWindowType {
  MAIN,
  PEER,
  GROUP,
  OTHERS
};

/**
 * @note 鉴于本类成员函数所访问的(CoreThread)类成员链表都具有只增不减的特性，
 * 所以无须加锁访问，若有例外，请于注释中说明，否则应当bug处理.\n
 * 若此特性不可被如此利用,请报告bug.
 */
class MainWindow {
 public:
  MainWindow(GtkApplication* app, IptuxConfig &config, ProgramData &progdt);
  ~MainWindow();

  GtkWidget* getWindow();

  void CreateWindow();
  void AlterWindowMode();
  GtkWidget *ObtainWindow();

  bool PaltreeContainItem(in_addr_t ipv4);
  void UpdateItemToPaltree(in_addr_t ipv4);
  void AttachItemToPaltree(in_addr_t ipv4);
  void DelItemFromPaltree(in_addr_t ipv4);
  void ClearAllItemFromPaltree();
  void MakeItemBlinking(GroupInfo *grpinf, bool blinking);
  void setActiveWindow(ActiveWindowType t, void* activeWindow);

  void OpenTransWindow();
  void UpdateItemToTransTree(GData **para);
  bool TransmissionActive();

  ProgramData &GetProgramData() { return progdt; }

  IptuxConfig &getConfig() { return config; }

  void SetStatusIcon(StatusIcon *statusIcon) { this->statusIcon = statusIcon; }

 private:
  GtkApplication* app;
  GtkWidget* window;
  IptuxConfig &config;
  ProgramData &progdt;
  StatusIcon *statusIcon;

  GData *widset;  //窗体集
  GData *mdlset;  //数据model集
  GList *tmdllist;       // model链表，用于构建model循环结构
  GtkAccelGroup *accel;  //快捷键集组
  guint timerid;         // UI更新定时器ID
  WindowConfig windowConfig;

  ActiveWindowType activeWindowType;
  void* activeWindow;

 private:
  void InitSublayer();
  void ClearSublayer();

  GtkWidget *CreateMainWindow();
  GtkWidget *CreateTransWindow();
  GtkWidget *CreateAllArea();
  GtkWidget *CreateTransArea();

  GtkWidget *CreateToolBar();
  GtkWidget *CreatePaltreeArea();
  GtkWidget *CreatePallistArea();

  GtkTreeModel *CreatePaltreeModel();
  GtkTreeModel *CreatePallistModel();
  GtkTreeModel *CreateTransModel();
  GtkWidget *CreatePaltreeTree(GtkTreeModel *model);
  GtkWidget *CreatePallistTree(GtkTreeModel *model);
  GtkWidget *CreateTransTree(GtkTreeModel *model);

  bool GroupGetPrevPaltreeItem(GtkTreeModel *model, GtkTreeIter *iter,
                               GroupInfo *grpinf);
  bool GroupGetPaltreeItem(GtkTreeModel *model, GtkTreeIter *iter,
                           GroupInfo *grpinf);
  bool GroupGetPaltreeItemWithParent(GtkTreeModel *model, GtkTreeIter *iter,
                                     GroupInfo *grpinf);
  void FillGroupInfoToPaltree(GtkTreeModel *model, GtkTreeIter *iter,
                              GroupInfo *grpinf);
  void UpdateGroupInfoToPaltree(GtkTreeModel *model, GtkTreeIter *iter,
                                GroupInfo *grpinf);
  void BlinkGroupItemToPaltree(GtkTreeModel *model, GtkTreeIter *iter,
                               bool blinking);
  static GtkWidget *CreateTransPopupMenu(GtkTreeModel *model);
  static GtkWidget *CreatePaltreePopupMenu(GroupInfo *grpinf);
  static void FillPalInfoToBuffer(GtkTextBuffer *buffer, PalInfo *pal);

 public:
  static void ShowTransWindow(MainWindow* self);

 private:
  //回调处理部分
  static gboolean UpdateUI(MainWindow *mwin);
  static void GoPrevTreeModel(MainWindow *mwin);
  static void GoNextTreeModel(MainWindow *mwin);

  static gboolean UpdateTransUI(GtkWidget *treeview);
  static gboolean TransPopupMenu(GtkWidget *treeview, GdkEventButton *event);
  static void HideTransWindow(GData **widset);
  static void ClearTransWindow(GData **widset);
  static void TerminateTransTask(GtkTreeModel *model);
  static void TerminateAllTransTask(GtkTreeModel *model);
  static void ClearTransTask(GtkTreeModel *model);
  static void OpenContainingFolder(GtkTreeModel *model);
  static void OpenThisFile(GtkTreeModel *model);

  static void AskSharedFiles(GroupInfo *grpinf);
  static void DeletePalItem(GroupInfo *grpinf);
  static gboolean PaltreeQueryTooltip(GtkWidget *treeview, gint x, gint y,
                                      gboolean key, GtkTooltip *tooltip,
                                      MainWindow *self);
  static void onPaltreeItemActivated(GtkWidget *treeview, GtkTreePath *path,
                                     GtkTreeViewColumn *column,
                                     MainWindow *self);
  static gboolean PaltreePopupMenu(GtkWidget *treeview, GdkEventButton *event);
  static void onPaltreePopupMenuSendMessageActivateRegular(
      GroupInfo *groupInfo);
  static void onPaltreePopupMenuSendMessageActivateGroup(GroupInfo *groupInfo);
  static gboolean PaltreeChangeStatus(GtkWidget *treeview,
                                      GdkEventButton *event);
  static void PaltreeDragDataReceived(GtkWidget *treeview,
                                      GdkDragContext *context, gint x, gint y,
                                      GtkSelectionData *data, guint info,
                                      guint time, MainWindow *self);

  static gint PaltreeCompareByNameFunc(GtkTreeModel *model, GtkTreeIter *a,
                                       GtkTreeIter *b);
  static gint PaltreeCompareByIPFunc(GtkTreeModel *model, GtkTreeIter *a,
                                     GtkTreeIter *b);

  static void ShowPallistArea(GData **widset);
  static void HidePallistArea(GData **widset);
  static gboolean ClearPallistEntry(GtkWidget *entry, GdkEventKey *event);
  static void PallistEntryChanged(GtkWidget *entry, GData **widset);
  static void PallistItemActivated(GtkWidget *treeview, GtkTreePath *path,
                                   GtkTreeViewColumn *column, MainWindow *self);
  static void PallistDragDataReceived(GtkWidget *treeview,
                                      GdkDragContext *context, gint x, gint y,
                                      GtkSelectionData *data, guint info,
                                      guint time, MainWindow *self);

  static gboolean MWinConfigureEvent(GtkWidget *window,
                                     GdkEventConfigure *event,
                                     MainWindow *self);
  static gboolean TWinConfigureEvent(GtkWidget *window,
                                     GdkEventConfigure *event,
                                     MainWindow *self);
  static void PanedDivideChanged(GtkWidget *paned, GParamSpec *pspec,
                                 MainWindow *self);
  static gboolean onDeleteEvent(MainWindow *self);
  static void onRefresh (void *, void *, MainWindow& self);
  static void onDetect (void *, void *, MainWindow& self);
  static void onFind (void *, void *, MainWindow& self);
  static void onAbout (void *, void *, MainWindow& self);
  static void onClearChatHistory (void *, void *, MainWindow& self);
  static void onSortType (void *, GVariant* value, MainWindow& self);
  static void onSortBy (void *, GVariant* value, MainWindow& self);
  static void onActive(MainWindow& self);
};

}  // namespace iptux

#endif
