//
// C++ Interface: DialogPeer
//
// Description:
// 与单个好友对话
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DIALOGPEER_H
#define DIALOGPEER_H

#include "mess.h"

class DialogPeer: public SessionAbstract {
public:
	DialogPeer(GroupInfo *grp);
	virtual ~DialogPeer();

	static void PeerDialogEntry(GroupInfo *grpinf);

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
	GtkWidget *CreateInfoArea();
	GtkWidget *CreateEnclosureArea();
	GtkWidget *CreateHistoryArea();
	GtkWidget *CreateInputArea();

	GtkTreeModel *CreateEnclosureModel();
	GtkWidget *CreateEnclosureTree(GtkTreeModel *model);

	GtkWidget *CreateFileMenu();
	GtkWidget *CreateToolMenu();
	GtkWidget *CreateHelpMenu();

	void FillPalInfoToBuffer(GtkTextBuffer *buffer, PalInfo *pal);
	GSList *PickEnclosure(uint32_t fileattr);

	GData *widset;		//窗体集
	GData *mdlset;		//数据model集
	GData *dtset;		//通用数据集
	GtkAccelGroup *accel;	//快捷键组
	GroupInfo *grpinf;	//群组信息
private:
	bool SendEnclosureMsg();
	bool SendTextMsg();
	void FeedbackMsg(const GSList *dtlist);
	MsgPara *PackageMsg(GSList *dtlist);
//回调处理部分
private:
	static void DragDataReceived(DialogPeer *dlgpr, GdkDragContext *context,
					 gint x, gint y, GtkSelectionData *data,
					 guint info, guint time);
	static void DragPicReceived(DialogPeer *dlgpr, GdkDragContext *context,
					 gint x, gint y, GtkSelectionData *data,
					 guint info, guint time);
	static void AttachRegular(DialogPeer *dlgpr);
	static void AttachFolder(DialogPeer *dlgpr);
	static void AskSharedFiles(GroupInfo *grpinf);
	static void InsertPicture(DialogPeer *dlgpr);
	static void ClearHistoryBuffer(DialogPeer *dlgpr);
	static void SendMessage(DialogPeer *dlgpr);

	static gboolean WindowConfigureEvent(GtkWidget *window,
				 GdkEventConfigure *event, GData **dtset);
	static void PanedDivideChanged(GtkWidget *paned, GParamSpec *pspec,
							 GData **dtset);
	static void DialogPeerDestroy(DialogPeer *dlgpr);
//线程处理
private:
	static void ThreadSendTextMsg(MsgPara *para);
};

#endif
