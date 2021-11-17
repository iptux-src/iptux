#ifndef IPTUX_GROUP_INFO_MANAGER_H
#define IPTUX_GROUP_INFO_MANAGER_H

#include "iptux/GroupInfo.h"
#include "iptux/LogSystem.h"

namespace iptux {

class UiCoreThread;
class GroupInfoManager {
 public:
  GroupInfoManager(UiCoreThread* coreThread, LogSystem_S logSystem);

  void addGroupInfo(GroupInfo_S groupInfo);

  GroupInfo_S addPal(PalInfo_S pal, PalInfo_SC me);
  GroupInfo_S addGroup(GroupBelongType type, PalInfo_SC me, std::string name);

  GroupInfo_S getGroupInfo(const PalInfo* pal);
  GroupInfo_S getGroupInfo(const GroupInfo::KeyType& key);

 private:
  UiCoreThread* core_thread_;
  LogSystem_S logSystem;
  std::map<GroupInfo::KeyType, GroupInfo_S> groupInfos;
};

using GroupInfoManager_U = std::unique_ptr<GroupInfoManager>;

}  // namespace iptux

#endif
