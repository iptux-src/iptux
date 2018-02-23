#ifndef IPTUX_EVENT_H
#define IPTUX_EVENT_H

#include "Models.h"
namespace iptux {

enum class EventType {
  NEW_PAL_ONLINE,
};

class Event {
 public:
  Event() = default;
  virtual ~Event() = default;

  virtual EventType getType() const = 0;
};

class NewPalOnlineEvent:public Event {
 public:
  explicit NewPalOnlineEvent(PalInfo* palInfo);
  EventType getType() const override ;

  const PalInfo* getPalInfo() const;
 private:
  PalInfo* palInfo;
};

}

#endif //IPTUX_EVENT_H
