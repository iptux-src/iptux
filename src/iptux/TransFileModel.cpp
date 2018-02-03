//
// Created by LIDaobing on 2018-02-03.
//

#include "TransFileModel.h"

#include <glib.h>

namespace iptux {
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

TransFileModel& TransFileModel::setFileLength(const std::string& value) {
  fileLength = value;
  return *this;
}

TransFileModel& TransFileModel::setFinishLength(const std::string& value) {
  finishLength = value;
  return *this;
}

TransFileModel& TransFileModel::setProgress(double value) {
  progress = value;
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

TransFileModel& TransFileModel::setData(TransAbstract* value) {
  data = value;
  return *this;
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

const std::string& TransFileModel::getFileLength() const {
  return fileLength;
}

const std::string& TransFileModel::getFinishLength() const {
  return finishLength;
}

double TransFileModel::getProgress() const {
  return progress;
}

const std::string TransFileModel::getProgressText() const {
  const char* t = g_strdup_printf("%.1f", progress);
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

TransAbstract* TransFileModel::getData() const {
  return data;
}

}