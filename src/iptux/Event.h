#ifndef IPTUX_EVENT_H
#define IPTUX_EVENT_H

#include "Models.h"
namespace iptux {

enum class EventType {
  NEW_PAL_ONLINE,
  NEW_MESSAGE,
};

class Event {
 public:
  explicit Event(EventType type);
  virtual ~Event() = default;

  EventType getType() const;
  virtual Event* clone() const = 0;
 private:
  EventType type;
};

class NewPalOnlineEvent:public Event {
 public:
  explicit NewPalOnlineEvent(PalInfo* palInfo);
  const PalInfo* getPalInfo() const;
  NewPalOnlineEvent* clone() const override;
 private:
  PalInfo* palInfo;
};

class NewMessageEvent:public Event {
 public:
  explicit NewMessageEvent(MsgPara&& msgPara);
  const MsgPara& getMsgPara() const;
  NewMessageEvent* clone() const override;
 private:
  MsgPara msgPara;
};

}

#endif //IPTUX_EVENT_H
