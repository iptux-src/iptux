#ifndef IPTUX_EVENT_H
#define IPTUX_EVENT_H

#include "iptux-core/Models.h"

namespace iptux {

enum class EventType {
  NEW_PAL_ONLINE,
  PAL_UPDATE,
  PAL_OFFLINE,
  NEW_MESSAGE,
  ICON_UPDATE,
  PASSWORD_REQUIRED,
  PERMISSION_REQUIRED,
  NEW_SHARE_FILE_FROM_FRIEND,
  SEND_FILE_STARTED,
  SEND_FILE_FINISHED,
  RECV_FILE_STARTED,
  RECV_FILE_FINISHED,
  TRANS_TASKS_CHANGED,
  CONFIG_CHANGED,
};

const char* EventTypeToStr(EventType type);

class Event {
 public:
  explicit Event(EventType type);
  virtual ~Event() = default;

  EventType getType() const;
  virtual std::string getSource() const;

 private:
  EventType type;
};

class PalEvent : public Event {
 public:
  explicit PalEvent(PalKey palKey, EventType type)
      : Event(type), palKey(palKey) {}
  const PalKey& GetPalKey() const { return palKey; }
  std::string getSource() const override;

 private:
  PalKey palKey;
};

class NewPalOnlineEvent : public PalEvent {
 public:
  explicit NewPalOnlineEvent(CPPalInfo palInfo);
  CPPalInfo getPalInfo() const;

 private:
  CPPalInfo palInfo;
};

class PalUpdateEvent : public PalEvent {
 public:
  explicit PalUpdateEvent(CPPalInfo palInfo);
  CPPalInfo getPalInfo() const;

 private:
  CPPalInfo palInfo;
};

class NewMessageEvent : public PalEvent {
 public:
  explicit NewMessageEvent(MsgPara&& msgPara);
  const MsgPara& getMsgPara() const;

 private:
  MsgPara msgPara;
};

class PalOfflineEvent : public PalEvent {
 public:
  explicit PalOfflineEvent(PalKey palKey);
};

class IconUpdateEvent : public PalEvent {
 public:
  explicit IconUpdateEvent(PalKey palKey)
      : PalEvent(palKey, EventType::ICON_UPDATE) {}
};

class PasswordRequiredEvent : public PalEvent {
 public:
  explicit PasswordRequiredEvent(PalKey palKey)
      : PalEvent(palKey, EventType::PASSWORD_REQUIRED) {}
};

class PermissionRequiredEvent : public PalEvent {
 public:
  explicit PermissionRequiredEvent(PalKey palKey)
      : PalEvent(palKey, EventType::PERMISSION_REQUIRED) {}
};

class NewShareFileFromFriendEvent : public PalEvent {
 public:
  explicit NewShareFileFromFriendEvent(PalKey palKey, FileInfo fileInfo)
      : PalEvent(palKey, EventType::NEW_SHARE_FILE_FROM_FRIEND), fileInfo(fileInfo) {}
  const FileInfo& GetFileInfo() const { return fileInfo; }

 private:
  FileInfo fileInfo;
};

class AbstractTaskIdEvent : public Event {
 protected:
  AbstractTaskIdEvent(EventType et, int taskId) : Event(et), taskId(taskId) {}

 public:
  int GetTaskId() const { return taskId; }

 private:
  int taskId;
};

class SendFileStartedEvent : public AbstractTaskIdEvent {
 public:
  explicit SendFileStartedEvent(int taskId)
      : AbstractTaskIdEvent(EventType::SEND_FILE_STARTED, taskId) {}
};

class SendFileFinishedEvent : public AbstractTaskIdEvent {
 public:
  explicit SendFileFinishedEvent(int taskId)
      : AbstractTaskIdEvent(EventType::SEND_FILE_FINISHED, taskId) {}
};

class RecvFileStartedEvent : public AbstractTaskIdEvent {
 public:
  explicit RecvFileStartedEvent(int taskId)
      : AbstractTaskIdEvent(EventType::RECV_FILE_STARTED, taskId) {}
};

class RecvFileFinishedEvent : public AbstractTaskIdEvent {
 public:
  explicit RecvFileFinishedEvent(int taskId)
      : AbstractTaskIdEvent(EventType::RECV_FILE_FINISHED, taskId) {}
};

class TransTasksChangedEvent : public Event {
 public:
  TransTasksChangedEvent() : Event(EventType::TRANS_TASKS_CHANGED) {}
};

class ConfigChangedEvent : public Event {
 public:
  ConfigChangedEvent() : Event(EventType::CONFIG_CHANGED) {}
};

}  // namespace iptux

#endif  // IPTUX_EVENT_H
