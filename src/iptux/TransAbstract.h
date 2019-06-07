#ifndef IPTUX_TRANSABSTRACT_H
#define IPTUX_TRANSABSTRACT_H

#include "iptux/TransFileModel.h"

namespace iptux {
/**
 * 传输抽象类.
 * 提供文件传输类必需的公共接口.
 */
class TransAbstract {
 public:
  TransAbstract();
  virtual ~TransAbstract();

  virtual const TransFileModel& getTransFileModel() const = 0;  ///< 获取更新UI的数据
  virtual void TerminateTrans() = 0;       ///< 终止过程处理
  int GetTaskId();
  void SetTaskId(int taskId);
 private:
  int taskId;
};

}

#endif //IPTUX_TRANSABSTRACT_H
