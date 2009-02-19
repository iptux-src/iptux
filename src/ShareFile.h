//
// C++ Interface: ShareFile
//
// Description:添加或删除共享文件,即管理共享文件
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SHAREFILE_H
#define SHAREFILE_H

#include "face.h"
#include "sys.h"

class ShareFile {
 public:
	ShareFile();
	~ShareFile();

	static void ShareEntry();
 private:
	void InitShare();
	void CreateShare();
	void AddSharedFiles(GSList * list);
	void FindInsertPosition(const gchar * path, uint32_t fileattr,
				GtkTreeIter * iter);
	GtkTreeModel *CreateSharedModel();
	GtkWidget *CreateSharedView();
	static bool CheckExist();

	GtkWidget *share_view;
	GtkTreeModel *share_model;
	static GtkWidget *share;
 private:
	void PickFile(uint32_t fileattr);
//回调处理部分
 private:
	static void AddRegular(gpointer data);	//ShareFile
	static void AddFolder(gpointer data);	//
	static void DeleteFiles(gpointer data);	//
	static void ClickOk(gpointer data);	//
	static void ClickApply(gpointer data);	//
	static void ShareDestroy(gpointer data);	//
	static void DragDataReceived(gpointer data, GdkDragContext * context,
				     gint x, gint y, GtkSelectionData * select,
				     guint info, guint time);	//

	static void SetPasswd();
	static void ClearPasswd();
};

#endif
