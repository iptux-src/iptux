#include "config.h"
#include "GroupInfoManager.h"

#include "iptux-utils/utils.h"

namespace iptux {

GroupInfoManager::GroupInfoManager(UiProgramData_S programData,
                                   LogSystem_S logSystem)
    : programData(programData), logSystem(logSystem) {}

GroupInfo_S GroupInfoManager::addPal(PalInfo_S pal, PalInfo_SC me) {
  auto grpinf = make_shared<GroupInfo>(pal, me, logSystem);
  grpinf->grpid = inAddrToUint32(pal->ipv4);
  grpinf->name = pal->getName();
  grpinf->buffer = gtk_text_buffer_new(programData->table);
  grpinf->clearDialog();
  addGroupInfo(grpinf);
  return grpinf;
}

GroupInfo_S GroupInfoManager::getGroupInfo(const PalInfo* pal) {
  return groupInfos[pal->GetKey()];
}

void GroupInfoManager::addGroupInfo(GroupInfo_S groupInfo) {
  groupInfos[groupInfo->getKey()] = groupInfo;
}

}  // namespace iptux
