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
#ifndef IPTUX_RECVFILEDATA_H
#define IPTUX_RECVFILEDATA_H

#include <gio/gio.h>

#include "iptux-core/CoreThread.h"
#include "iptux-core/Models.h"
#include "iptux-core/internal/TransAbstract.h"
#include "iptux-core/internal/ipmsg.h"

namespace iptux {

class RecvFileData : public TransAbstract {
 public:
  RecvFileData(CoreThread* coreThread, FileInfo* fl);
  virtual ~RecvFileData();

  void RecvFileDataEntry();
  virtual const TransFileModel& getTransFileModel() const;
  virtual void TerminateTrans();

 private:
  void CreateUIPara();
  void RecvRegularFile();
  void RecvDirFiles();

  int64_t RecvData(int sock, int fd, int64_t filesize, int64_t offset);
  int64_t RecvData(GSocket* sock, int fd, int64_t filesize, int64_t offset);
  void UpdateUIParaToOver();

  CoreThread* coreThread;
  FileInfo* file;  //文件信息
  TransFileModel para;
  bool terminate;                     //终止标志(也作处理结果标识)
  int64_t sumsize;                    //文件(目录)总大小
  char buf[MAX_SOCKLEN];              //数据缓冲区
  struct timeval tasktime, filetime;  //任务开始时间&文件开始时间
};

}  // namespace iptux

#endif
