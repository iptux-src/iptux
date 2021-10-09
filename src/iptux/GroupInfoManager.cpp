#include "config.h"
#include "GroupInfoManager.h"

#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

GroupInfoManager::GroupInfoManager(UiProgramData_S programData,
                                   LogSystem_S logSystem)
    : programData(programData), logSystem(logSystem) {}

GroupInfo_S GroupInfoManager::addPal(PalInfo_S pal, PalInfo_SC me) {
  GroupInfo_S grpinf(new GroupInfo(pal, me, logSystem));
  grpinf->grpid = inAddrToUint32(pal->ipv4);
  grpinf->name = pal->getName();
  grpinf->buffer = gtk_text_buffer_new(programData->table);
  grpinf->clearDialog();
  addGroupInfo(grpinf);
  return grpinf;
}

GroupInfo_S GroupInfoManager::getGroupInfo(const GroupInfo::KeyType& key) {
  return groupInfos[key];
}

GroupInfo_S GroupInfoManager::addGroup(GroupBelongType type,
                                       PalInfo_SC me,
                                       std::string name) {
  GroupInfo_S grpinf(new GroupInfo(type, vector<PPalInfo>(), me, logSystem));
  grpinf->grpid = g_quark_from_static_string(name.c_str());
  grpinf->name = name;
  grpinf->buffer = gtk_text_buffer_new(programData->table);
  grpinf->clearDialog();
  addGroupInfo(grpinf);
  return grpinf;
}

GroupInfo_S GroupInfoManager::getGroupInfo(const PalInfo* pal) {
  return groupInfos[GroupInfo::genKey(pal)];
}

void GroupInfoManager::addGroupInfo(GroupInfo_S groupInfo) {
  groupInfos[groupInfo->getKey()] = groupInfo;
}

}  // namespace iptux
