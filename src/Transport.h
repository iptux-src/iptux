//
// C++ Interface: Transport
//
// Description:传输文件数据,必要时创建传输框
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TRANSPORT_H
#define TRANSPORT_H

#include "face.h"
#include "sys.h"

class Transport {
 public:
	Transport();
	~Transport();

	void InitSelf();
	bool TransportActive();
	static void TransportEntry();
	static void RecvFileEntry(GtkTreeIter * iter);
	static void SendFileEntry(int sock, GtkTreeIter * iter,
				  uint32_t fileattr);
 private:
	 GtkTreeModel * CreateTransModel();
	bool CheckExist();
	void CreateTransView();
	void CreateTransDialog();
	void RecvFileData(GtkTreeIter * iter);
	void RecvDirFiles(GtkTreeIter * iter);
	uint32_t RecvData(int sock, int fd, GtkTreeIter * iter,
			  uint64_t filesize, char *buf, uint32_t offset);
	void SendFileData(int sock, GtkTreeIter * iter);
	void SendDirFiles(int sock, GtkTreeIter * iter);
	uint32_t SendData(int sock, int fd, GtkTreeIter * iter,
			  uint64_t filesize, char *buf);
	void EndTransportData(int sock, int fd, GtkTreeIter * iter,
			      const char *pathname);
	void EndTransportDirFiles(GtkTreeIter * iter, char *filename,
				  uint64_t finishsize, uint64_t filesize);

	GtkWidget *transport;
	GtkWidget *trans_view;
	GtkTreeModel *trans_model;
	GtkTreeIter opt_iter;
	bool flag;
 public:
	inline GtkTreeModel *TransModelQuote() {
		return trans_model;
	}
 private:
	 GtkWidget * CreatePopupMenu();
//回调处理部分
 public:
 private:
	static void DestroyDialog();
	static gboolean PopupControlMenu(GtkWidget * view,
			 GdkEventButton * event, gpointer data);	//Transport
	static void StopTask(gpointer data);	//
	static void StopAllTask(gpointer data);	//
	static void TidyTask(gpointer data);	//
};

#endif
