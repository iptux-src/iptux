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

#include "iptux-core/Event.h"
#include "iptux-core/IptuxConfig.h"
#include "iptux-core/Models.h"

#include "iptux/Application.h"
#include "iptux/EventAdaptor.h"
#include "iptux/UiCoreThread.h"
#include "iptux/UiModels.h"
#include "iptux/WindowConfig.h"

namespace iptux {

class StatusIcon;

/**
 * @note 鉴于本类成员函数所访问的(CoreThread)类成员链表都具有只增不减的特性，
 * 所以无须加锁访问，若有例外，请于注释中说明，否则应当bug处理.\n
 * 若此特性不可被如此利用,请报告bug.
 */
class MainWindow : public sigc::trackable {
 public:
  MainWindow(Application* app, UiCoreThread& coreThread);
  ~MainWindow();

  GtkWidget* getWindow();

  void AlterWindowMode();

  bool PaltreeContainItem(in_addr ipv4);
  void UpdateItemToPaltree(in_addr ipv4);
  void AttachItemToPaltree(in_addr ipv4);
  void DelItemFromPaltree(in_addr ipv4);
  void ClearAllItemFromPaltree();

  std::shared_ptr<ProgramData> GetProgramData() { return progdt; }

  std::shared_ptr<IptuxConfig> getConfig() { return config; }

  Application* getApp() { return app; }

  PalTreeModelSortKey sort_key() const { return sort_key_; }
  GtkSortType sort_type() const { return sort_type_; }
  GroupInfoStyle info_style() const { return info_style_; }

 private:
  Application* app;
  UiCoreThread& coreThread;
  GtkWidget* window;
  EventAdaptor* eventAdaptor;

  std::shared_ptr<ProgramData> progdt;
  std::shared_ptr<IptuxConfig> config;

  GData* widset;  // 窗体集
  GData* mdlset;  // 数据model集
  PalTreeModel* regular_model = 0;

  GList* tmdllist;  // model链表，用于构建model循环结构
  guint timerid;    // UI更新定时器ID
  WindowConfig windowConfig;
  GtkBuilder* builder;
  GtkMenu* palPopupMenu;

  GroupInfo* currentGroupInfo = 0;
  GtkSortType sort_type_ = GTK_SORT_ASCENDING;
  PalTreeModelSortKey sort_key_ = PalTreeModelSortKey::NICKNAME;
  GroupInfoStyle info_style_ = GroupInfoStyle::IP;

 private:
  void setCurrentGroupInfo(GroupInfo* groupInfo);

  void InitSublayer();
  void LoadConfig();
  void SaveConfig();
  void ClearSublayer();

  void CreateWindow();
  GtkWidget* CreateMainWindow();
  void CreateActions();
  void CreateTitle();
  GtkWidget* CreateAllArea();

  GtkWidget* CreateToolBar();
  GtkWidget* CreatePaltreeArea();
  GtkWidget* CreatePallistArea();

  GtkTreeModel* CreatePallistModel();
  GtkWidget* CreatePaltreeTree(GtkTreeModel* model);
  GtkWidget* CreatePallistTree(GtkTreeModel* model);

  /**
   * @brief refresh pal list, used when change view options.
   */
  void RefreshPalList();
  void RefreshPalListRegular();
  bool GroupGetPrevPaltreeItem(GtkTreeModel* model,
                               GtkTreeIter* iter,
                               GroupInfo* grpinf);
  bool GroupGetPaltreeItem(GtkTreeModel* model,
                           GtkTreeIter* iter,
                           GroupInfo* grpinf);
  bool GroupGetPaltreeItemWithParent(GtkTreeModel* model,
                                     GtkTreeIter* iter,
                                     GroupInfo* grpinf);
  void FillGroupInfoToPaltree(GtkTreeModel* model,
                              GtkTreeIter* iter,
                              GroupInfo* grpinf);
  void BlinkGroupItemToPaltree(GtkTreeModel* model,
                               GtkTreeIter* iter,
                               bool blinking);
  static void FillPalInfoToBuffer(GtkTextBuffer* buffer, PalInfo* pal);
  void processEventInMainThread(std::shared_ptr<const Event> event);

 private:
  std::string getTitle() const;
  static gboolean UpdateUI(MainWindow* mwin);
  static void GoPrevTreeModel(MainWindow* mwin);
  static void GoNextTreeModel(MainWindow* mwin);

  void DeletePalItem(GroupInfo* grpinf);
  static gboolean PaltreeQueryTooltip(GtkWidget* treeview,
                                      gint x,
                                      gint y,
                                      gboolean key,
                                      GtkTooltip* tooltip,
                                      MainWindow* self);
  static void onPaltreeItemActivated(GtkWidget* treeview,
                                     GtkTreePath* path,
                                     GtkTreeViewColumn* column,
                                     MainWindow* self);
  static gboolean PaltreePopupMenu(GtkWidget* treeview,
                                   GdkEventButton* event,
                                   MainWindow* self);
  static gboolean PaltreeChangeStatus(GtkWidget* treeview,
                                      GdkEventButton* event);
  static void PaltreeDragDataReceived(GtkWidget* treeview,
                                      GdkDragContext* context,
                                      gint x,
                                      gint y,
                                      GtkSelectionData* data,
                                      guint info,
                                      guint time,
                                      MainWindow* self);
  static void HidePallistArea(GData** widset);
  static gboolean ClearPallistEntry(GtkWidget* entry, GdkEventKey* event);
  static void PallistEntryChanged(GtkWidget* entry, MainWindow* self);
  static void PallistItemActivated(GtkWidget* treeview,
                                   GtkTreePath* path,
                                   GtkTreeViewColumn* column,
                                   MainWindow* self);
  static void PallistDragDataReceived(GtkWidget* treeview,
                                      GdkDragContext* context,
                                      gint x,
                                      gint y,
                                      GtkSelectionData* data,
                                      guint info,
                                      guint time,
                                      MainWindow* self);

  static gboolean MWinConfigureEvent(GtkWidget* window,
                                     GdkEventConfigure* event,
                                     MainWindow* self);
  static gboolean TWinConfigureEvent(GtkWidget* window,
                                     GdkEventConfigure* event,
                                     MainWindow* self);
  static void PanedDivideChanged(GtkWidget* paned,
                                 GParamSpec* pspec,
                                 MainWindow* self);
  static void onRefresh(void*, void*, MainWindow& self);
  static void onDetect(void*, void*, MainWindow& self);
  static void onFind(void*, void*, MainWindow& self);
  static void onPalSendMessage(void*, void*, MainWindow& self);
  static void onPalRequestSharedResources(void*, void*, MainWindow& self);
  static void onPalChangeInfo(void*, void*, MainWindow& self);
  static void onDeletePal(void*, void*, MainWindow& self);
  static void onSortType(GSimpleAction* action,
                         GVariant* value,
                         MainWindow& self);
  static void onSortBy(GSimpleAction* action,
                       GVariant* value,
                       MainWindow& self);
  static void onInfoStyle(GSimpleAction* action,
                          GVariant* value,
                          MainWindow& self);
  static gboolean onNewPalOnlineEvent(gpointer data);
  void onGroupInfoUpdated(GroupInfo* groupInfo);
};

}  // namespace iptux

#endif
