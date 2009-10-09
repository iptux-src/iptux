//
// C++ Interface: RecvFileData
//
// Description:
// 接收文件数据
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef RECVFILEDATA_H
#define RECVFILEDATA_H

#include "mess.h"

class RecvFileData: public TransAbstract
{
public:
	RecvFileData(FileInfo *fl);
	~RecvFileData();

	void RecvFileDataEntry();
	virtual GData **GetTransFilePara();
	virtual void TerminateTrans();
private:
	void CreateUIPara();
	void RecvRegularFile();
	void RecvDirFiles();

	int64_t RecvData(int sock, int fd, int64_t filesize, int64_t offset);
	void UpdateUIParaToOver();

	FileInfo *file;		//文件信息
	GData *para;		//UI参考数据
	bool terminate;		//终止标志(也作处理结果标识)
	int64_t sumsize;	//文件(目录)总大小
	char buf[MAX_SOCKLEN];	//数据缓冲区
	struct timeval tasktime, filetime;	//任务开始时间&文件开始时间
};

#endif
