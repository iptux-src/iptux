#ifndef IPTUX_GROUP_INFO_MANAGER_H
#define IPTUX_GROUP_INFO_MANAGER_H

#include "iptux/GroupInfo.h"
#include "iptux/LogSystem.h"
#include "iptux/UiProgramData.h"

namespace iptux {

class GroupInfoManager {
 public:
  GroupInfoManager(UiProgramData_S programData, LogSystem_S logSystem);

  void addGroupInfo(GroupInfo_S groupInfo);
  GroupInfo_S addPal(PalInfo_S pal, PalInfo_SC me);
  GroupInfo_S getGroupInfo(const PalInfo* pal);

 private:
  UiProgramData_S programData;
  LogSystem_S logSystem;
  std::map<GroupInfo::KeyType, GroupInfo_S> groupInfos;
};

using GroupInfoManager_U = std::unique_ptr<GroupInfoManager>;

}  // namespace iptux

#endif
