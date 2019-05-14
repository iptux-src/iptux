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
#include "iptux/UiProgramData.h"
#include "iptux/WindowConfig.h"
#include "iptux/Models.h"
#include "iptux/RecvFileData.h"
#include "iptux/UiModels.h"
#include "iptux/Event.h"
#include "iptux/UiCoreThread.h"

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
  MainWindow(GtkApplication* app, UiCoreThread& coreThread);
  ~MainWindow();

  GtkWidget* getWindow();

  void CreateWindow();
  void AlterWindowMode();

  bool PaltreeContainItem(in_addr_t ipv4);
  void UpdateItemToPaltree(in_addr_t ipv4);
  void AttachItemToPaltree(in_addr_t ipv4);
  void DelItemFromPaltree(in_addr_t ipv4);
  void ClearAllItemFromPaltree();
  void MakeItemBlinking(GroupInfo *grpinf, bool blinking);
  void setActiveWindow(ActiveWindowType t, void* activeWindow);
  void clearActiveWindow(void* activeWindow);

  void OpenTransWindow();
  //void UpdateItemToTransTree(GData **para);
  void UpdateItemToTransTree(const TransFileModel& para);
  bool isTransmissionActive() const;

  std::shared_ptr<UiProgramData> GetProgramData() { return progdt; }

  IptuxConfig &getConfig() { return config; }

  void SetStatusIcon(StatusIcon *statusIcon) { this->statusIcon = statusIcon; }

  void processEvent(const Event& event);

 private:
  GtkApplication* app;
  UiCoreThread& coreThread;
  GtkWidget* window;
  GtkWidget* transWindow;

  std::shared_ptr<UiProgramData> progdt;
  IptuxConfig &config;
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
  GtkWidget *CreateAllArea();

  GtkWidget *CreateToolBar();
  GtkWidget *CreatePaltreeArea();
  GtkWidget *CreatePallistArea();

  GtkTreeModel *CreatePallistModel();
  GtkWidget *CreatePaltreeTree(GtkTreeModel *model);
  GtkWidget *CreatePallistTree(GtkTreeModel *model);

  bool GroupGetPrevPaltreeItem(GtkTreeModel *model, GtkTreeIter *iter,
                               GroupInfo *grpinf);
  bool GroupGetPaltreeItem(GtkTreeModel *model, GtkTreeIter *iter,
                           GroupInfo *grpinf);
  bool GroupGetPaltreeItemWithParent(GtkTreeModel *model, GtkTreeIter *iter,
                                     GroupInfo *grpinf);
  void FillGroupInfoToPaltree(GtkTreeModel *model, GtkTreeIter *iter,
                              GroupInfo *grpinf);
  void BlinkGroupItemToPaltree(GtkTreeModel *model, GtkTreeIter *iter,
                               bool blinking);
  static GtkWidget *CreatePaltreePopupMenu(GroupInfo *grpinf);
  static void FillPalInfoToBuffer(GtkTextBuffer *buffer, PalInfo *pal);
  void InitThemeSublayerData();
  void processEventInMainThread(Event* event);

 private:
  //回调处理部分
  static gboolean processEventCallback(gpointer data);
  static gboolean UpdateUI(MainWindow *mwin);
  static void GoPrevTreeModel(MainWindow *mwin);
  static void GoNextTreeModel(MainWindow *mwin);

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
  static void onInsertPicture (void *, void *, MainWindow& self);
  static void onSortType (GSimpleAction *action, GVariant* value, MainWindow& self);
  static void onSortBy (GSimpleAction *action, GVariant* value, MainWindow& self);
  static void onActive(MainWindow& self);
  static gboolean onNewPalOnlineEvent(gpointer data);
};

}  // namespace iptux

#endif
