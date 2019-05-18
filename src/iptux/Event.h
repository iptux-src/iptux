#ifndef IPTUX_EVENT_H
#define IPTUX_EVENT_H

#include "Models.h"
namespace iptux {

enum class EventType {
  NEW_PAL_ONLINE,
  NEW_MESSAGE,
  PAL_OFFLINE,
};

class Event {
 public:
  explicit Event(EventType type);
  virtual ~Event() = default;

  EventType getType() const;
 private:
  EventType type;
};

class NewPalOnlineEvent:public Event {
 public:
  explicit NewPalOnlineEvent(PPalInfo palInfo);
  CPPalInfo getPalInfo() const;
 private:
  PPalInfo palInfo;
};

class NewMessageEvent:public Event {
 public:
  explicit NewMessageEvent(MsgPara&& msgPara);
  const MsgPara& getMsgPara() const;
 private:
  MsgPara msgPara;
};

class PalOfflineEvent: public Event {
 public:
  explicit PalOfflineEvent(PalKey palKey);
  const PalKey& GetPalKey() const;
 private:
  PalKey palKey;
};

}

#endif //IPTUX_EVENT_H
