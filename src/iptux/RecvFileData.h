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

#include "iptux/ipmsg.h"
#include "iptux/mess.h"

namespace iptux {

class RecvFileData;

class RecvFileDataPara {
 public:
  RecvFileDataPara& setStatus(const std::string& value);
  RecvFileDataPara& setTask(const std::string& value);
  RecvFileDataPara& setPeer(const std::string& value);
  RecvFileDataPara& setIp(const std::string& value);
  RecvFileDataPara& setFilename(const std::string& value);
  RecvFileDataPara& setFileLength(const std::string& value);
  RecvFileDataPara& setFinishLength(const std::string& value);
  RecvFileDataPara& setProgress(double value);
  RecvFileDataPara& setCost(const std::string& value);
  RecvFileDataPara& setRemain(const std::string& value);
  RecvFileDataPara& setRate(const std::string& value);
  RecvFileDataPara& setFilePath(const std::string& value);
  RecvFileDataPara& setData(RecvFileData* value);

  const std::string& getStatus() const;
  const std::string& getTask() const;
  const std::string& getPeer() const;
  const std::string& getIp() const;
  const std::string& getFilename() const;
  const std::string& getFileLength() const;
  const std::string& getFinishLength() const;
  double getProgress() const;
  const std::string getProgressText() const;
  const std::string& getCost() const;
  const std::string& getRemain() const;
  const std::string& getRate() const;
  const std::string& getFilePath() const;
  RecvFileData* getData() const;
 private:
  std::string status;
  std::string task;
  std::string peer;
  std::string ip;
  std::string filename;
  std::string fileLength;
  std::string finishLength;
  double progress;
  std::string cost;
  std::string remain;
  std::string rate;
  std::string filePath;
  RecvFileData* data;
};

class RecvFileData{
 public:
  RecvFileData(FileInfo *fl);
  ~RecvFileData();

  void RecvFileDataEntry();
  const RecvFileDataPara& GetTransFilePara() const ;
  void TerminateTrans();

 private:
  void CreateUIPara();
  void RecvRegularFile();
  void RecvDirFiles();

  int64_t RecvData(int sock, int fd, int64_t filesize, int64_t offset);
  void UpdateUIParaToOver();

  FileInfo *file;                     //文件信息
  RecvFileDataPara para;
  bool terminate;                     //终止标志(也作处理结果标识)
  int64_t sumsize;                    //文件(目录)总大小
  char buf[MAX_SOCKLEN];              //数据缓冲区
  struct timeval tasktime, filetime;  //任务开始时间&文件开始时间
};


}  // namespace iptux

#endif
