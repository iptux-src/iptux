//
// C++ Interface: DialogPeer
//
// Description:好友对话框
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DIALOGPEER_H
#define DIALOGPEER_H

#include "face.h"
#include "Pal.h"

class DialogPeer {
 public:
	DialogPeer(gpointer data);	//Pal
	~DialogPeer();

	static void DialogEntry(gpointer data);	//
 private:
	void CreateDialog();
	void CreateAllArea();
	void CreateInfoArea(GtkWidget * paned);
	void CreateRecordArea(GtkWidget * paned);
	void CreateInputArea(GtkWidget * paned);
	GtkWidget *CreateMenuBar();
	void CreateFileMenu(GtkWidget * menu_bar);
	void CreateToolMenu(GtkWidget * menu_bar);
	void CreateHelpMenu(GtkWidget * menu_bar);
	static bool CheckExist(gpointer data);	//

	GtkWidget *dialog;	//主窗口
	GtkWidget *focus;	//焦点
	GtkWidget *scroll;	//滚动
	GtkTextBuffer *infobuf;
	GtkAccelGroup *accel;
	Pal *pal;
 public:
	inline GtkWidget *DialogQuote() {
		return dialog;
	} inline GtkWidget *ScrollQuote() {
		return scroll;
	}

	static void FillPalInfoToBuffer(gpointer data, GtkTextBuffer * buffer,
					bool sad = true);	//
//回调处理部分
 public:
	static void DragDataReceived(gpointer data, GdkDragContext * context,
				     gint x, gint y, GtkSelectionData * select,
				     guint info, guint time);	//
	static void AskSharedFiles(gpointer data);	//
 private:
	static void DragPicReceived(GtkWidget * view, GdkDragContext * context,
				    gint x, gint y, GtkSelectionData * select,
				    guint info, guint time,
				    GtkTextBuffer * buffer);
	static void DialogDestroy(gpointer data);	//DialogPeer
	static void InsertPixbuf(gpointer data);	//
    static void ClearRecordBuffer(Pal *palobj);
	static void SendMessage(gpointer data);	//
//线程处理
 private:
	static void ThreadSendMessage(gpointer data);	//struct sendmsg_para
};

#endif
