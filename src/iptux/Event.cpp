#include "config.h"
#include "Event.h"

using namespace std;

namespace iptux {

Event::Event(EventType type)
  : type(type) {}

EventType Event::getType() const {
  return type;
}

NewPalOnlineEvent::NewPalOnlineEvent(PalInfo *palInfo)
  : Event(EventType::NEW_PAL_ONLINE),
    palInfo(palInfo) {}

const PalInfo* NewPalOnlineEvent::getPalInfo() const {
  return palInfo;
}

NewPalOnlineEvent* NewPalOnlineEvent::clone() const {
  return new NewPalOnlineEvent(palInfo);
}

NewMessageEvent::NewMessageEvent(MsgPara&& msgPara)
  : Event(EventType::NEW_MESSAGE),
    msgPara(msgPara) {}

const MsgPara& NewMessageEvent::getMsgPara() const {
  return msgPara;
}

NewMessageEvent* NewMessageEvent::clone() const {
  MsgPara para = msgPara;
  return new NewMessageEvent(move(para));
}


}

