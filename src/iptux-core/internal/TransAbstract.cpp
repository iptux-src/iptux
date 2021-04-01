#include "config.h"
#include "TransAbstract.h"

namespace iptux {
TransAbstract::TransAbstract() {}
TransAbstract::~TransAbstract() {}

void TransAbstract::SetTaskId(int taskId) {
  this->taskId = taskId;
}

int TransAbstract::GetTaskId() {
  return taskId;
}

}  // namespace iptux
