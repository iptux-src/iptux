#include "config.h"
#include "iptux-core/Event.h"

using namespace std;

namespace iptux {

Event::Event(EventType type)
  : type(type) {}

EventType Event::getType() const {
  return type;
}

NewPalOnlineEvent::NewPalOnlineEvent(PPalInfo palInfo)
  : Event(EventType::NEW_PAL_ONLINE),
    palInfo(palInfo) {}

CPPalInfo NewPalOnlineEvent::getPalInfo() const {
  return palInfo;
}

NewMessageEvent::NewMessageEvent(MsgPara&& msgPara)
  : Event(EventType::NEW_MESSAGE),
    msgPara(msgPara) {}

const MsgPara& NewMessageEvent::getMsgPara() const {
  return msgPara;
}

PalOfflineEvent::PalOfflineEvent(PalKey palKey)
  : Event(EventType::PAL_OFFLINE),
    palKey(move(palKey)) {}

const PalKey& PalOfflineEvent::GetPalKey() const {
  return palKey;
}

}
