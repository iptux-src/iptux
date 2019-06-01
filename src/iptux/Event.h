#ifndef IPTUX_EVENT_H
#define IPTUX_EVENT_H

#include "Models.h"
namespace iptux {

enum class EventType {
  NEW_PAL_ONLINE,
  NEW_MESSAGE,
  PAL_OFFLINE,
  ICON_UPDATE,
  PASSWORD_REQUIRED,
  PERMISSION_REQUIRED,
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

class IconUpdateEvent: public Event {
 public:
  explicit IconUpdateEvent(PalKey palKey): Event(EventType::ICON_UPDATE), palKey(palKey) {}
  const PalKey& GetPalKey() const {return palKey;}
 private:
  PalKey palKey;
};

class PasswordRequiredEvent: public Event {
 public:
  explicit PasswordRequiredEvent(PalKey palKey):
    Event(EventType::PASSWORD_REQUIRED),
    palKey(palKey) {}
  const PalKey& GetPalKey() const {return palKey;}
 private:
  PalKey palKey;
};

class PermissionRequiredEvent: public Event {
 public:
  explicit PermissionRequiredEvent(PalKey palKey):
    Event(EventType::PERMISSION_REQUIRED),
    palKey(palKey) {}
  const PalKey& GetPalKey() const {return palKey;}
 private:
  PalKey palKey;
};
}

#endif //IPTUX_EVENT_H
