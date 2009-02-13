//
// C++ Interface: MainWindow
//
// Description:创建主面板
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "face.h"
#include "net.h"

class MainWindow {
 private:
	static const char *localip[];
 public:
	MainWindow();
	~MainWindow();

	void CreateWindow();

	bool PalGetModelIter(gpointer data, GtkTreeIter * iter);	//Pal
	void AttachItemToModel(in_addr_t ipv4, GtkTreeIter * iter);
	void SetValueToModel(gpointer data, GtkTreeIter * iter);	//
	void DelItemFromModel(gpointer data);	//
 private:
	 void InitPanel();
	 void CreatePanel();
	 void CreateAllArea();
	 GtkWidget * CreateMenuBar();
	GtkWidget *CreatePalTree();
	GtkTreeModel * CreatePalTreeModel();
	void InitPalTreeModel();
	void Ipv4GetParent(in_addr_t ipv4, GtkTreeIter * parent);

	void CreateFileMenu(GtkWidget * menu_bar);
	void CreateToolMenu(GtkWidget * menu_bar);
	void CreateHelpMenu(GtkWidget * menu_bar);

	GtkWidget *window;
	GtkWidget *client_paned;
	GtkWidget *tips;
	GtkWidget *pal_tree;
	GtkTreeModel *tree_model;
	GtkAccelGroup *accel;
	GtkTreeIter opt_iter;

 public:
    void        UpdateStyle();
	static void UpdateTips();
 private:
	 GtkWidget * CreatePopupPalMenu(gpointer data);	//
	GtkWidget *CreatePopupSectionMenu();
	GtkTreeModel *CreatePalListModel();
	GtkWidget *CreatePalListView();
//回调处理部分
 private:
	static void UpdatePalTree(gpointer data);	//MainWindow
	static void DeletePal(gpointer data);	//Pal
	static gboolean TreeQueryTooltip(GtkWidget * view, gint x, gint y,
					 gboolean key, GtkTooltip * tooltip,
					 GtkTreeModel * model);
	static void TreeItemActivated(GtkWidget * view, GtkTreePath * path,
				      GtkTreeViewColumn * column,
				      GtkTreeModel * model);
	static gboolean TreePopupMenu(GtkWidget * view, GdkEventButton * event,
				      gpointer data);	//MainWindow
	static gboolean TreeChangeStatus(GtkWidget * view, GdkEventButton * event,
					 gpointer data);	//
	static void TreeItemChangeStatus(gpointer data);	//
	static void TreeDragDataReceived(GtkWidget * view,
					 GdkDragContext * context, gint x,
					 gint y, GtkSelectionData * select,
					 guint info, guint time,
					 GtkTreeModel * model);

	static void SortSectionByIP(gpointer data);	//
	static void SortSectionByNickname(gpointer data);
	static void SortSectionByGroup(gpointer data);
	static void SortSectionByCommunication(gpointer data);

	static void AttachFindArea(gpointer data);	//
	static gboolean ClearFindEntry(GtkWidget * entry, GdkEventKey * event);
	static void FindEntryChanged(GtkWidget * entry, GtkWidget * view);
	static void ListItemActivated(GtkWidget * view, GtkTreePath * path,
				      GtkTreeViewColumn * column,
				      GtkTreeModel * model);
	static gboolean ListPopupMenu(GtkWidget * view, GdkEventButton * event,
				      gpointer data);	//
	static void ListDragDataReceived(GtkWidget * view,
					 GdkDragContext * context, gint x,
					 gint y, GtkSelectionData * select,
					 guint info, guint time,
					 GtkTreeModel * model);
};

#endif
