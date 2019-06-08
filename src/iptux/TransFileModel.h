#ifndef IPTUX_TRANSFILEMODEL_H
#define IPTUX_TRANSFILEMODEL_H

#include <string>

namespace iptux {

class TransFileModel {
 public:
  TransFileModel();

  TransFileModel &setStatus(const std::string &value);
  TransFileModel &setTask(const std::string &value);
  TransFileModel &setPeer(const std::string &value);
  TransFileModel &setIp(const std::string &value);
  TransFileModel &setFilename(const std::string &value);
  TransFileModel &setFileLength(int64_t value);
  TransFileModel &setFinishedLength(int64_t value);
  TransFileModel &setCost(const std::string &value);
  TransFileModel &setRemain(const std::string &value);
  TransFileModel &setRate(const std::string &value);
  TransFileModel &setFilePath(const std::string &value);
  TransFileModel &setTaskId(int taskId);
  void finish();

  const std::string &getStatus() const;
  const std::string &getTask() const;
  const std::string &getPeer() const;
  const std::string &getIp() const;
  const std::string &getFilename() const;
  int64_t getFileLength() const;
  std::string getFileLengthText() const;
  std::string getFinishedLengthText() const;
  double getProgress() const;
  std::string getProgressText() const;
  const std::string &getCost() const;
  const std::string &getRemain() const;
  const std::string &getRate() const;
  const std::string &getFilePath() const;
  bool isFinished() const;
  int getTaskId() const;

 private:
  std::string status;
  std::string task;
  std::string peer;
  std::string ip;
  std::string filename;
  int64_t fileLength;
  int64_t finishedLength;
  std::string cost;
  std::string remain;
  std::string rate;
  std::string filePath;
  bool finished;
  int taskId;
};

}

#endif //IPTUX_TRANSFILEMODEL_H
