#ifndef IPTUX_TRANSFILEMODEL_H
#define IPTUX_TRANSFILEMODEL_H

#include <string>

namespace iptux {

class TransAbstract;

class TransFileModel {
 public:
  TransFileModel &setStatus(const std::string &value);
  TransFileModel &setTask(const std::string &value);
  TransFileModel &setPeer(const std::string &value);
  TransFileModel &setIp(const std::string &value);
  TransFileModel &setFilename(const std::string &value);
  TransFileModel &setFileLength(int64_t value);
  TransFileModel &setFinishedLength(const std::string &value);
  TransFileModel &setProgress(double value);
  TransFileModel &setCost(const std::string &value);
  TransFileModel &setRemain(const std::string &value);
  TransFileModel &setRate(const std::string &value);
  TransFileModel &setFilePath(const std::string &value);
  TransFileModel &setData(TransAbstract *value);

  const std::string &getStatus() const;
  const std::string &getTask() const;
  const std::string &getPeer() const;
  const std::string &getIp() const;
  const std::string &getFilename() const;
  std::string getFileLengthText() const;
  const std::string &getFinishLength() const;
  double getProgress() const;
  std::string getProgressText() const;
  const std::string &getCost() const;
  const std::string &getRemain() const;
  const std::string &getRate() const;
  const std::string &getFilePath() const;
  TransAbstract *getData() const;
 private:
  std::string status;
  std::string task;
  std::string peer;
  std::string ip;
  std::string filename;
  int64_t fileLength;
  std::string finishLength;
  double progress;
  std::string cost;
  std::string remain;
  std::string rate;
  std::string filePath;
  TransAbstract *data;
};

}

#endif //IPTUX_TRANSFILEMODEL_H
