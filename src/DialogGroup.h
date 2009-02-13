//
// C++ Interface: DialogGroup
//
// Description:群发消息
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DIALOGGROUP_H
#define DIALOGGROUP_H

#include "face.h"

class DialogGroup {
 public:
	DialogGroup();
	~DialogGroup();

	static void DialogEntry();
 private:
	void InitDialog();
	void CreateDialog();
	GtkWidget *CreateMenuBar();
	void CreateChooseArea(GtkWidget * paned);
	void CreateRecordArea(GtkWidget * paned);
	void CreateInputArea(GtkWidget * paned);
	GtkTreeModel *CreateGroupModel();
	void InitGroupModel();
	GtkWidget *CreateGroupView();
	void CreateFileMenu(GtkWidget * menu_bar);
	void CreateHelpMenu(GtkWidget * menu_bar);
	static bool CheckExist();

	GtkWidget *pal_view;
	GtkWidget *record, *input;
	GtkAccelGroup *accel;
	GtkTreeModel *group_model;
	static GtkWidget *dialog;
 private:
	void BufferInsertText(const gchar * msg);
	void SendGroupMsg(const gchar * msg);
	void ViewScroll();
	static GtkWidget *CreatePopupMenu(GtkTreeModel * model);
//回调处理部分
 public:
	/*参数 model:0 为toggle项 */
	static void ViewToggleChange(GtkTreeModel * model, gchar * path);
	static gboolean PopupPickMenu(GtkTreeModel * model,
					      GdkEventButton * event);
 private:
	static void SendMessage(gpointer data);	//DialogGroup
	static void UpdatePalList(gpointer data);	//
	static void DialogDestroy(gpointer data);	//

	static void SelectAll(GtkTreeModel * model);
	static void TurnSelect(GtkTreeModel * model);
	static void ClearAll(GtkTreeModel * model);
};

#endif
