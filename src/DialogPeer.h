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
#include "DialogBase.h"

class DialogPeer: public DialogBase {
public:
	DialogPeer(GroupInfo *grp);
	virtual ~DialogPeer();

	static void PeerDialogEntry(GroupInfo *grpinf);

	virtual void UpdatePalData(PalInfo *pal);
	virtual void InsertPalData(PalInfo *pal);
	virtual void DelPalData(PalInfo *pal);
	virtual void ClearAllPalData();
private:
	void ReadUILayout();
	void WriteUILayout();

	GtkWidget *CreateMainWindow();
	GtkWidget *CreateAllArea();

	GtkWidget *CreateMenuBar();
	GtkWidget *CreateInfoArea();

	GtkWidget *CreateToolMenu();

	void FillPalInfoToBuffer(GtkTextBuffer *buffer, PalInfo *pal);

private:
	bool SendEnclosureMsg();
	bool SendTextMsg();
	void FeedbackMsg(const GSList *dtlist);
	MsgPara *PackageMsg(GSList *dtlist);
//回调处理部分
private:
	static void DragPicReceived(DialogPeer *dlgpr, GdkDragContext *context,
					 gint x, gint y, GtkSelectionData *data,
					 guint info, guint time);
	static void AskSharedFiles(GroupInfo *grpinf);
	static void InsertPicture(DialogPeer *dlgpr);

	static void DialogPeerDestroy(DialogPeer *dlgpr);
//线程处理
private:
	static void ThreadSendTextMsg(MsgPara *para);
};

#endif
