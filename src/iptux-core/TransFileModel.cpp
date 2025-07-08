#include "config.h"
#include "iptux-core/TransFileModel.h"
#include "iptux-utils/utils.h"

#include <glib.h>

namespace iptux {

TransFileModel::TransFileModel()
    : fileLength(0), finishedLength(0), finished(false) {}

TransFileModel& TransFileModel::setStatus(const std::string& value) {
  status = value;
  return *this;
}

TransFileModel& TransFileModel::setTask(const std::string& value) {
  task = value;
  return *this;
}

TransFileModel& TransFileModel::setPeer(const std::string& value) {
  peer = value;
  return *this;
}

TransFileModel& TransFileModel::setIp(const std::string& value) {
  ip = value;
  return *this;
}

TransFileModel& TransFileModel::setFilename(const std::string& value) {
  filename = value;
  return *this;
}

TransFileModel& TransFileModel::setFileLength(int64_t value) {
  fileLength = value;
  return *this;
}

TransFileModel& TransFileModel::setFinishedLength(int64_t value) {
  finishedLength = value;
  return *this;
}

TransFileModel& TransFileModel::setCost(const std::string& value) {
  cost = value;
  return *this;
}

TransFileModel& TransFileModel::setRemain(const std::string& value) {
  remain = value;
  return *this;
}

TransFileModel& TransFileModel::setRate(const std::string& value) {
  rate = value;
  return *this;
}

TransFileModel& TransFileModel::setFilePath(const std::string& value) {
  filePath = value;
  return *this;
}

TransFileModel& TransFileModel::setTaskId(int taskId) {
  this->taskId = taskId;
  return *this;
}

void TransFileModel::finish() {
  finished = true;
}

const std::string& TransFileModel::getStatus() const {
  return status;
}

const std::string& TransFileModel::getTask() const {
  return task;
}

const std::string& TransFileModel::getPeer() const {
  return peer;
}

const std::string& TransFileModel::getIp() const {
  return ip;
}

const std::string& TransFileModel::getFilename() const {
  return filename;
}

std::string TransFileModel::getFileLengthText() const {
  const char* t = numeric_to_size(fileLength);
  std::string res(t);
  g_free(gpointer(t));
  return res;
}

std::string TransFileModel::getFinishedLengthText() const {
  const char* t = numeric_to_size(finishedLength);
  std::string res(t);
  g_free(gpointer(t));
  return res;
}

double TransFileModel::getProgress() const {
  if (fileLength <= 0) {
    return 0.0;  // Avoid division by zero
  }
  return percent(finishedLength, fileLength);
}

std::string TransFileModel::getProgressText() const {
  const char* t = g_strdup_printf("%.1f", getProgress());
  std::string res(t);
  g_free(gpointer(t));
  return res;
}

const std::string& TransFileModel::getCost() const {
  return cost;
}

const std::string& TransFileModel::getRemain() const {
  return remain;
}

const std::string& TransFileModel::getRate() const {
  return rate;
}

const std::string& TransFileModel::getFilePath() const {
  return filePath;
}

int64_t TransFileModel::getFileLength() const {
  return fileLength;
}

bool TransFileModel::isFinished() const {
  return finished;
}

int TransFileModel::getTaskId() const {
  return taskId;
}

}  // namespace iptux
