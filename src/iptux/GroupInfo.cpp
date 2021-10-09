#include "config.h"
#include "GroupInfo.h"

#include "iptux-utils/output.h"
#include "iptux/DialogBase.h"

using namespace std;

namespace iptux {

GroupInfo::GroupInfo(iptux::GroupBelongType t,
                     const vector<PPalInfo>& pals,
                     CPPalInfo me,
                     LogSystem_S logSystem)
    : grpid(0),
      buffer(NULL),
      dialogBase(NULL),
      me(me),
      members(pals),
      type(t),
      logSystem(logSystem) {
  inputBuffer = gtk_text_buffer_new(NULL);
}

GroupInfo::GroupInfo(PPalInfo pal, CPPalInfo me, LogSystem_S logSystem)
    : grpid(0),
      buffer(NULL),
      dialogBase(NULL),
      me(me),
      type(GROUP_BELONG_TYPE_REGULAR),
      logSystem(logSystem) {
  members.push_back(pal);
  inputBuffer = gtk_text_buffer_new(NULL);
}

GroupInfo::~GroupInfo() {
  g_object_unref(buffer);
}

bool GroupInfo::hasPal(PalInfo* pal) const {
  for (auto i : members) {
    if (i.get() == pal) {
      return true;
    }
  }
  return false;
}

bool GroupInfo::hasPal(PPalInfo pal) const {
  return hasPal(pal.get());
}

GtkWidget* GroupInfo::getDialog() const {
  return dialogBase ? GTK_WIDGET(dialogBase->getWindow()) : nullptr;
}

PalKey GroupInfo::getKey() const {
  if (type == GROUP_BELONG_TYPE_REGULAR) {
    return getMembers()[0]->GetKey();
  }
  throw std::runtime_error("GroupInfo::getKey()");
}

bool GroupInfo::addPal(PPalInfo pal) {
  if (type == GROUP_BELONG_TYPE_REGULAR) {
    LOG_WARN("should not call addPal on GROUP_BELONG_TYPE_REGULAR");
    return false;
  }
  if (hasPal(pal)) {
    return false;
  }
  members.push_back(pal);
  return true;
}

bool GroupInfo::delPal(PalInfo* pal) {
  if (type == GROUP_BELONG_TYPE_REGULAR) {
    LOG_WARN("should not call delPal on GROUP_BELONG_TYPE_REGULAR");
    return false;
  }

  for (auto it = members.begin(); it != members.end(); ++it) {
    if (it->get() == pal) {
      members.erase(it);
      return true;
    }
  }
  return false;
}

void GroupInfo::newFileReceived() {
  this->signalNewFileReceived.emit(this);
}

void GroupInfo::addMsgCount(int i) {
  int oldCount = getUnreadMsgCount();
  allMsgCount += i;
  signalUnreadMsgCountUpdated.emit(this, oldCount, getUnreadMsgCount());
}

void GroupInfo::readAllMsg() {
  int oldCount = getUnreadMsgCount();
  if (oldCount != 0) {
    readMsgCount = allMsgCount;
    signalUnreadMsgCountUpdated.emit(this, oldCount, getUnreadMsgCount());
  }
}

int GroupInfo::getUnreadMsgCount() const {
  g_assert(allMsgCount >= readMsgCount);
  return allMsgCount - readMsgCount;
}
}  // namespace iptux
