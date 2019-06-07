#include "config.h"
#include "TransAbstract.h"

namespace iptux {
TransAbstract::TransAbstract() {}
TransAbstract::~TransAbstract() {}

  // void SetTaskId(int taskId);

int TransAbstract::GetTaskId() {
  return taskId;
}

}
