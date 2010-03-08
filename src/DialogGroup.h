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
#ifndef DIALOGGROUP_H
#define DIALOGGROUP_H

#include "mess.h"

class DialogGroup: public SessionAbstract {
public:
	DialogGroup(GroupInfo *grp);
	virtual ~DialogGroup();

	static void GroupDialogEntry(GroupInfo *grpinf);

	virtual void UpdatePalData(PalInfo *pal);
	virtual void InsertPalData(PalInfo *pal);
	virtual void DelPalData(PalInfo *pal);
	virtual void ClearAllPalData();
	virtual void ShowEnclosure();
	virtual void AttachEnclosure(const GSList *list);
	virtual void ScrollHistoryTextview();
private:
	void InitSublayer();
	void ClearSublayer();
	void ReadUILayout();
	void WriteUILayout();
	void ClearHistoryTextView();

	GtkWidget *CreateMainWindow();
	GtkWidget *CreateAllArea();

	GtkWidget *CreateMenuBar();
	GtkWidget *CreateMemberArea();
	GtkWidget *CreateEnclosureArea();
	GtkWidget *CreateHistoryArea();
	GtkWidget *CreateInputArea();

	GtkTreeModel *CreateMemberModel();
	GtkTreeModel *CreateEnclosureModel();
	void FillMemberModel(GtkTreeModel *model);
	GtkWidget *CreateMemberTree(GtkTreeModel *model);
	GtkWidget *CreateEnclosureTree(GtkTreeModel *model);

	GtkWidget *CreateFileMenu();
	GtkWidget *CreateToolMenu();
	GtkWidget *CreateHelpMenu();

	GSList *PickEnclosure(uint32_t fileattr);

	GData *widset;		//窗体集
	GData *mdlset;		//数据model集
	GData *dtset;		//通用数据集
	GtkAccelGroup *accel;	//快捷键组
	GroupInfo *grpinf;	//群组信息
private:
	bool SendEnclosureMsg();
	bool SendTextMsg();
	void FeedbackMsg(const gchar *msg);
	void BroadcastEnclosureMsg(GSList *list);
	void BroadcastTextMsg(const gchar *msg);

	static GtkWidget *CreatePopupMenu(GtkTreeModel *model);
//回调处理部分
private:
	static gint MemberTreeCompareByNameFunc(GtkTreeModel *model,
					 GtkTreeIter *a, GtkTreeIter *b);
	static gint MemberTreeCompareByIPFunc(GtkTreeModel *model,
					 GtkTreeIter *a, GtkTreeIter *b);
	static void SetMemberTreeSortFunc(GtkWidget *menuitem, GtkTreeModel *model);
	static void SetMemberTreeSortType(GtkWidget *menuitem, GtkTreeModel *model);
	static void DragDataReceived(DialogGroup *dlggrp, GdkDragContext *context,
					 gint x, gint y, GtkSelectionData *data,
					 guint info, guint time);
	static gboolean PopupPickMenu(GtkWidget *treeview, GdkEventButton *event);
	static void MembertreeItemActivated(GtkWidget *treeview, GtkTreePath *path,
							 GtkTreeViewColumn *column);
	static void AttachRegular(DialogGroup *dlggrp);
	static void AttachFolder(DialogGroup *dlggrp);
	static void ClearHistoryBuffer(DialogGroup *dlggrp);
	static void SendMessage(DialogGroup *dlggrp);

	static gboolean WindowConfigureEvent(GtkWidget *window,
				 GdkEventConfigure *event, GData **dtset);
	static void PanedDivideChanged(GtkWidget *paned, GParamSpec *pspec,
							 GData **dtset);
	static void DialogGroupDestroy(DialogGroup *dlggrp);
};

#endif
