//
// C++ Interface: RecvFile
//
// Description:接受相关的文件信息,不包含文件数据
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef RECVFILE_H
#define RECVFILE_H

#include "Pal.h"

class RecvFile {
 public:
	RecvFile(gpointer data);	//struct recvfile_para
	~RecvFile();

	static void RecvEntry(gpointer data);	//
 private:
	void ParseExtra();
	void CreateRecvWindow();
	gpointer DivideFileinfo(char **ptr);
	GtkTreeModel *CreateRecvModel();
	GtkWidget *CreateRecvView();

	Pal *pal;
	char *msg;
	uint32_t commandn;
	uint32_t packetn;
	GSList *filelist;
	GtkTreeModel *file_model;
//回调处理部分
 private:
	static void CellEditText(GtkCellRendererText * renderer, gchar * path,
				 gchar * new_text, GtkTreeModel * model);
	static void CursorItemChanged(GtkWidget *view, GtkWidget *chooser);
	static void ChooserResetLabel(GtkWidget *chooser, GtkWidget *label);
	static void ChooserResetView(GtkWidget *chooser, GtkWidget *view);
	static void AdditionRecvFile(GtkTreeModel * model);
};

#endif
