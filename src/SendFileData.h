//
// C++ Interface: SendFileData
//
// Description:
// 发送文件数据
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SENDFILEDATA_H
#define SENDFILEDATA_H

#include "mess.h"

class SendFileData: public TransAbstract
{
public:
        SendFileData(int sk, FileInfo *fl);
        ~SendFileData();

        void SendFileDataEntry();
        virtual GData **GetTransFilePara();
        virtual void TerminateTrans();
private:
        void CreateUIPara();
        void SendRegularFile();
        void SendDirFiles();

        int64_t SendData(int fd, int64_t filesize);
        void UpdateUIParaToOver();

        int sock;               //数据套接口
        FileInfo *file;         //文件信息
        GData *para;            //UI参考数据
        bool terminate;         //终止标志(也作处理结果标识)
        int64_t sumsize;        //文件(目录)总大小
        char buf[MAX_SOCKLEN];  //数据缓冲区
        struct timeval tasktime, filetime;      //任务开始时间&文件开始时间
};

#endif
