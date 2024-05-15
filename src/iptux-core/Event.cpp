#include "config.h"
#include "iptux-core/Event.h"

using namespace std;

namespace iptux {

Event::Event(EventType type) : type(type) {}

static const char* event_type_strs[] = {
    [(int)EventType::NEW_PAL_ONLINE] = "NEW_PAL_ONLINE",
    [(int)EventType::PAL_UPDATE] = "PAL_UPDATE",
    [(int)EventType::PAL_OFFLINE] = "PAL_OFFLINE",
    [(int)EventType::NEW_MESSAGE] = "NEW_MESSAGE",
    [(int)EventType::ICON_UPDATE] = "ICON_UPDATE",
    [(int)EventType::PASSWORD_REQUIRED] = "PASSWORD_REQUIRED",
    [(int)EventType::PERMISSION_REQUIRED] = "PERMISSION_REQUIRED",
    [(int)EventType::NEW_SHARE_FILE_FROM_FRIEND] = "NEW_SHARE_FILE_FROM_FRIEND",
    [(int)EventType::SEND_FILE_STARTED] = "SEND_FILE_STARTED",
    [(int)EventType::SEND_FILE_FINISHED] = "SEND_FILE_FINISHED",
    [(int)EventType::RECV_FILE_STARTED] = "RECV_FILE_STARTED",
    [(int)EventType::RECV_FILE_FINISHED] = "RECV_FILE_FINISHED",
    [(int)EventType::TRANS_TASKS_CHANGED] = "TRANS_TASKS_CHANGED",
    [(int)EventType::CONFIG_CHANGED] = "CONFIG_CHANGED",
};

const char* EventTypeToStr(EventType type) {
  if (type < EventType::NEW_PAL_ONLINE || type > EventType::CONFIG_CHANGED) {
    return "UNKNOWN";
  }
  return event_type_strs[(int)type];
}

EventType Event::getType() const {
  return type;
}

string Event::getSource() const {
  return "NOT IMPLEMENTED";
}

string PalEvent::getSource() const {
  return GetPalKey().ToString();
}

NewPalOnlineEvent::NewPalOnlineEvent(CPPalInfo palInfo)
    : PalEvent(palInfo->GetKey(), EventType::NEW_PAL_ONLINE),
      palInfo(palInfo) {}

CPPalInfo NewPalOnlineEvent::getPalInfo() const {
  return palInfo;
}

PalUpdateEvent::PalUpdateEvent(CPPalInfo palInfo)
    : PalEvent(palInfo->GetKey(), EventType::PAL_UPDATE), palInfo(palInfo) {}

CPPalInfo PalUpdateEvent::getPalInfo() const {
  return palInfo;
}

NewMessageEvent::NewMessageEvent(MsgPara&& msgPara)
    : PalEvent(msgPara.getPal()->GetKey(), EventType::NEW_MESSAGE),
      msgPara(msgPara) {}

const MsgPara& NewMessageEvent::getMsgPara() const {
  return msgPara;
}

PalOfflineEvent::PalOfflineEvent(PalKey palKey)
    : PalEvent(palKey, EventType::PAL_OFFLINE) {}

}  // namespace iptux
