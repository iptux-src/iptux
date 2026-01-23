#ifndef IPTUX_CORETHREAD_H
#define IPTUX_CORETHREAD_H

#include "iptux-core/Models.h"
#include <atomic>
#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include <vector>

#include <sigc++/signal.h>

#include "iptux-core/Event.h"
#include "iptux-core/ProgramData.h"
#include "iptux-core/TransFileModel.h"

namespace iptux {

class TransAbstract;

enum CoreThreadErr {
  CORE_THREAD_ERR_NONE = 0,
  CORE_THREAD_ERR_STARTED_TWICE,
  CORE_THREAD_ERR_SOCKET_CREATE_FAILED,
  CORE_THREAD_ERR_UDP_BIND_FAILED,
  CORE_THREAD_ERR_UDP_THREAD_START_FAILED,
  CORE_THREAD_ERR_TCP_BIND_FAILED,
};

const char* coreThreadErrToStr(enum CoreThreadErr err);

class CoreThread {
 public:
  explicit CoreThread(std::shared_ptr<ProgramData> data);
  virtual ~CoreThread();

  virtual bool start() noexcept;
  virtual void stop();

  // For testing only: ignore TCP bind failures
  void setIgnoreTcpBindFailed(bool ignore);

  CPPalInfo getMe() const;
  PPalInfo getMe();

  int getTcpSock() const;
  int getUdpSock() const;
  uint16_t port() const;

  std::shared_ptr<ProgramData> getProgramData();
  bool BlacklistContainItem(in_addr ipv4) const;

  /**
   * @brief add ipaddress to block list
   *
   * @param ipv4 the ip address
   */
  void AddBlockIp(in_addr ipv4);

  /**
   * @brief whether the ipv4 address is blocked?
   *
   * @param ipv4: address
   * @return true if blocked
   * @return false if not blocked
   */
  bool IsBlocked(in_addr ipv4) const;

  void Lock() const;
  void Unlock() const;

  const std::vector<std::shared_ptr<PalInfo>>& GetPalList();
  virtual void ClearAllPalFromList();

  CPPalInfo GetPal(PalKey palKey) const;
  PPalInfo GetPal(PalKey palKey);
  CPPalInfo GetPal(in_addr ipv4) const { return GetPal(PalKey(ipv4, port())); }
  PPalInfo GetPal(in_addr ipv4) { return GetPal(PalKey(ipv4, port())); }
  CPPalInfo GetPal(const std::string& ipv4) const;
  PPalInfo GetPal(const std::string& ipv4);

  virtual void DelPalFromList(PalKey palKey);
  virtual void DelPalFromList(in_addr palKey) {
    DelPalFromList(PalKey(palKey, port()));
  }
  virtual void UpdatePalToList(PalKey palKey);
  virtual void UpdatePalToList(in_addr palKey) {
    UpdatePalToList(PalKey(palKey, port()));
  }

  virtual void AttachPalToList(PPalInfo pal);

  /**
   * @brief Get the directory path to store the received pal icon;
   *
   * @return std::string
   */
  std::string getUserIconPath() const;

  void AddPrivateFile(PFileInfo file);
  /**
   * return true if exist, return false if not exist.
   */
  bool DelPrivateFile(uint32_t id);
  PFileInfo GetPrivateFileById(uint32_t id);
  PFileInfo GetPrivateFileByPacketN(uint32_t packageNum, uint32_t filectime);

  bool sendFeatureData(PPalInfo pal) noexcept;
  void emitSomeoneExit(const PalKey& palKey);
  void emitNewPalOnline(PPalInfo palInfo);
  void emitNewPalOnline(const PalKey& palKey);
  void EmitIconUpdate(const PalKey& palKey);

  void emitEvent(std::shared_ptr<const Event> event);

  /**
   * @brief return event count since started
   *
   * @return int
   */
  int getEventCount() const;

  /**
   * @brief Get the Last Event object
   *
   * @return std::shared_ptr<const Event>
   */
  std::shared_ptr<const Event> getLastEvent() const;
  bool HasEvent() const;
  std::shared_ptr<const Event> PopEvent();

  const std::string& GetAccessPublicLimit() const;
  void SetAccessPublicLimit(const std::string& val);

  /**
   * @brief send message to pal
   *
   * @param pal
   * @param message string message
   * @return true if send success
   * @return false if send failed
   */
  bool SendMessage(CPPalInfo pal, const std::string& message);
  bool SendMessage(CPPalInfo pal, const ChipData& chipData);
  bool SendMsgPara(std::shared_ptr<MsgPara> msgPara);
  void AsyncSendMsgPara(std::shared_ptr<MsgPara> msgPara);
  void SendUnitMessage(const PalKey& palKey,
                       uint32_t opttype,
                       const std::string& message);
  void SendGroupMessage(const PalKey& palKey, const std::string& message);

  bool SendAskShared(PPalInfo pal);
  bool SendAskSharedWithPassword(const PalKey& palKey,
                                 const std::string& password);

  void SendDetectPacket(const std::string& ipv4);
  void SendDetectPacket(in_addr ipv4);
  void SendExit(PPalInfo pal);
  void SendMyIcon(PPalInfo pal, std::istream& iss);
  void SendSharedFiles(PPalInfo pal);

  void BcstFileInfoEntry(const std::vector<const PalInfo*>& pals,
                         const std::vector<FileInfo*>& files);

  /**
   * 插入消息(UI线程安全).
   * @param para 消息参数封装包
   * @note 消息数据必须使用utf8编码
   * @note (para->pal)不可为null
   * @note
   * 请不要关心函数内部实现，你只需要按照要求封装消息数据，然后扔给本函数处理就可以了，
   * 它会想办法将消息按照你所期望的格式插入到你所期望的TextBuffer，否则请发送Bug报告
   */
  void InsertMessage(const MsgPara& para);
  void InsertMessage(MsgPara&& para);

  void UpdateMyInfo();
  void SendBroadcastExit(PPalInfo pal);
  int GetOnlineCount() const;

  std::unique_ptr<TransFileModel> GetTransTaskStat(int taskId) const;
  std::vector<std::unique_ptr<TransFileModel>> listTransTasks() const;
  bool TerminateTransTask(int taskId);
  void clearFinishedTransTasks();

  void RecvFile(FileInfo* file);
  void RecvFileAsync(FileInfo* file);
  enum CoreThreadErr getLastErr() const;

 public:
  sigc::signal<void(std::shared_ptr<const Event>)> signalEvent;

  // these functions should be move to CoreThreadImpl
 public:
  void RegisterTransTask(std::shared_ptr<TransAbstract> task);

 public:
  static void SendNotifyToAll(CoreThread* pcthrd);

 protected:
  std::shared_ptr<ProgramData> programData;
  std::shared_ptr<IptuxConfig> config;
  mutable std::mutex mutex;  // 锁

 private:
  std::atomic_bool started;

 protected:
  virtual void ClearSublayer();

 private:
  bool bind_iptux_port() noexcept;

 private:
  static void RecvTcpData(CoreThread* pcthrd);

 public:
  struct Impl;

 private:
  std::unique_ptr<Impl> pImpl;
};

}  // namespace iptux

#endif  // IPTUX_CORETHREAD_H
