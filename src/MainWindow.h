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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mess.h"

/**
 * @note 鉴于本类成员函数所访问的(CoreThread)类成员链表都具有只增不减的特性，
 * 所以无须加锁访问，若有例外，请于注释中说明，否则应当bug处理.\n
 * 若此特性不可被如此利用,请报告bug.
 */
class MainWindow {
public:
        MainWindow();
        ~MainWindow();

        void CreateWindow();
        void AlterWindowMode();
        GtkWidget *ObtainWindow();

        bool PaltreeContainItem(in_addr_t ipv4);
        void UpdateItemToPaltree(in_addr_t ipv4);
        void AttachItemToPaltree(in_addr_t ipv4);
        void DelItemFromPaltree(in_addr_t ipv4);
        void ClearAllItemFromPaltree();
        void MakeItemBlinking(GroupInfo *grpinf, bool blinking);

        void OpenTransWindow();
        void UpdateItemToTransTree(GData **para);
        bool TransmissionActive();
private:
        void InitSublayer();
        void ClearSublayer();
        void ReadUILayout();
        void WriteUILayout();

        GtkWidget *CreateMainWindow();
        GtkWidget *CreateTransWindow();
        GtkWidget *CreateAllArea();
        GtkWidget *CreateTransArea();

        GtkWidget *CreateMenuBar();
        GtkWidget *CreateToolBar();
        GtkWidget *CreatePaltreeArea();
        GtkWidget *CreatePallistArea();

        GtkWidget *CreateFileMenu();
        GtkWidget *CreateToolMenu();
        GtkWidget *CreateHelpMenu();

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

        GData *widset;          //窗体集
        GData *mdlset;          //数据model集
        GData *dtset;           //通用数据集
        GList *tmdllist;                //model链表，用于构建model循环结构
        GtkAccelGroup *accel;   //快捷键集组
        guint timerid;          //UI更新定时器ID
private:
        static GtkWidget *CreateTransPopupMenu(GtkTreeModel *model);
        static GtkWidget *CreatePaltreePopupMenu(GroupInfo *grpinf);
        static void FillPalInfoToBuffer(GtkTextBuffer *buffer, PalInfo *pal);
//回调处理部分
private:
        static gboolean UpdateUI(MainWindow *mwin);
        static void GoPrevTreeModel(MainWindow *mwin);
        static void GoNextTreeModel(MainWindow *mwin);

        static gboolean UpdateTransUI(GtkWidget *treeview);
        static gboolean TransPopupMenu(GtkWidget *treeview, GdkEventButton *event);
        static void ShowTransWindow(GData **widset);
        static void HideTransWindow(GData **widset);
        static void ClearTransWindow(GData **widset);
        static void TerminateTransTask(GtkTreeModel *model);
        static void TerminateAllTransTask(GtkTreeModel *model);
        static void ClearTransTask(GtkTreeModel *model);
        static void OpenContainingFolder(GtkTreeModel *model);
        static void OpenThisFile(GtkTreeModel *model);

        static void UpdatePalTree(MainWindow *mwin);
        static void AskSharedFiles(GroupInfo *grpinf);
        static void DeletePalItem(GroupInfo *grpinf);
        static gboolean PaltreeQueryTooltip(GtkWidget *treeview, gint x, gint y,
                                         gboolean key, GtkTooltip *tooltip);
        static void PaltreeItemActivated(GtkWidget *treeview, GtkTreePath *path,
                                                 GtkTreeViewColumn *column);
        static gboolean PaltreePopupMenu(GtkWidget *treeview, GdkEventButton *event);
        static gboolean PaltreeChangeStatus(GtkWidget *treeview, GdkEventButton *event);
        static void PaltreeDragDataReceived(GtkWidget *treeview, GdkDragContext *context,
                                         gint x, gint y, GtkSelectionData *data,
                                         guint info, guint time);

        static gint PaltreeCompareByNameFunc(GtkTreeModel *model,
                                         GtkTreeIter *a, GtkTreeIter *b);
        static gint PaltreeCompareByIPFunc(GtkTreeModel *model,
                                         GtkTreeIter *a, GtkTreeIter *b);
        static void SetPaltreeSortFunc(GtkWidget *menuitem, GData **mdlset);
        static void SetPaltreeSortType(GtkWidget *menuitem, GData **mdlset);

        static void ShowPallistArea(GData **widset);
        static void HidePallistArea(GData **widset);
        static gboolean ClearPallistEntry(GtkWidget *entry, GdkEventKey *event);
        static void PallistEntryChanged(GtkWidget *entry,GData **widset);
        static void PallistItemActivated(GtkWidget *treeview, GtkTreePath *path,
                                                 GtkTreeViewColumn *column);
        static void PallistDragDataReceived(GtkWidget *treeview, GdkDragContext *context,
                                         gint x, gint y, GtkSelectionData *data,
                                         guint info, guint time);

        static gboolean MWinConfigureEvent(GtkWidget *window,
                                 GdkEventConfigure *event, GData **dtset);
        static gboolean TWinConfigureEvent(GtkWidget *window,
                                 GdkEventConfigure *event, GData **dtset);
        static void PanedDivideChanged(GtkWidget *paned, GParamSpec *pspec,
                                                         GData **dtset);
};

#endif
