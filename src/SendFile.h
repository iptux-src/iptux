//
// C++ Interface: SendFile
//
// Description:发送相关的文件信息,不包含文件数据
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SENDFILE_H
#define SENDFILE_H

#include "sys.h"
#include "face.h"

class SendFile {
 public:
	SendFile();
	~SendFile();

	void InitSelf();
	void WriteShared();

	static void SendRegular(gpointer data);	//Pal, 回调入口
	static void SendFolder(gpointer data);	//

	void RequestData(int sock, uint32_t fileattr, char *buf);
	void SendFileInfo(GSList * list, gpointer data);	//
	void SendSharedInfo(gpointer data);	//

	bool dirty;
 private:
	void PickFile(uint32_t fileattr, gpointer data);	//
	pointer FindFileinfo(uint32_t fileid);

	uint32_t pbn;
	GSList *pblist;
	char *passwd;	//共享文件密码, passwd != NULL
	uint32_t prn;
	GSList *prlist;
	pthread_mutex_t mutex;
 public:
	inline uint32_t &PbnQuote() {	//返回合适的编号
		return ++pbn;
	} inline GSList *&PblistQuote() {
		return pblist;
	} inline char *&PasswdQuote() {
		return passwd;
	}

	inline pthread_mutex_t *MutexQuote() {
		return &mutex;
	}
};

#endif
