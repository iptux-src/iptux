#ifndef IPTUX_GROUP_INFO_H
#define IPTUX_GROUP_INFO_H

#include <gtk/gtk.h>
#include <sigc++/signal.h>

#include "iptux-core/Models.h"
#include "iptux/LogSystem.h"

namespace iptux {
/**
 * 群组信息.
 */
class DialogBase;
class GroupInfoManager;
class GroupInfo {
 private:
  // only for friend class GroupInfoManager
  GroupInfo(PPalInfo pal, CPPalInfo me, LogSystem_S logSystem);

 public:
  GroupInfo(GroupBelongType type,
            const std::vector<PPalInfo>& pals,
            CPPalInfo me,
            LogSystem_S logSystem);

  ~GroupInfo();

  const std::vector<PPalInfo>& getMembers() const { return members; }
  GroupBelongType getType() const { return type; }

  using KeyType = std::pair<GroupBelongType, std::string>;
  KeyType getKey() const;
  static KeyType genKey(const PalInfo* pal);

  /** return true if successful added, noop for regular group */
  bool addPal(PPalInfo pal);

  /** return true if successful deleted, noop for regular group */
  bool delPal(PalInfo* pal);

  /** return true if successful deleted, noop for regulat group */
  bool delPal(PPalInfo pal);

  bool hasPal(PalInfo* pal) const;
  bool hasPal(PPalInfo pal) const;

  void addMsgPara(const MsgPara& msg);
  void readAllMsg();
  int getUnreadMsgCount() const;
  void newFileReceived();

  GtkTextBuffer* getInputBuffer() const { return inputBuffer; }

  void setDialogBase(DialogBase* dialogBase) { this->dialogBase = dialogBase; }
  GtkWidget* getDialog() const;
  void clearDialog() { dialogBase = nullptr; }

 public:
  sigc::signal<void(GroupInfo*, int, int)> signalUnreadMsgCountUpdated;
  sigc::signal<void(GroupInfo*)> signalNewFileReceived;

 public:
  GQuark grpid;           ///< 唯一标识
  std::string name;       ///< 群组名称 *
  GtkTextBuffer* buffer;  ///< 历史消息缓冲区 *

 private:
  DialogBase* dialogBase;
  GtkTextBuffer* inputBuffer;  /// 输入缓冲

 private:
  CPPalInfo me;
  std::vector<PPalInfo> members;
  GroupBelongType type;  ///< 群组类型
  LogSystem_S logSystem;
  int allMsgCount = 0;  /* all received message count */
  int readMsgCount = 0; /* already read message count */

 private:
  void addMsgCount(int i);
  friend GroupInfoManager;
};

using GroupInfo_S = std::shared_ptr<GroupInfo>;

}  // namespace iptux

#endif
