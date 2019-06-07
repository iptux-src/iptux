#ifndef IPTUX_CORETHREAD_H
#define IPTUX_CORETHREAD_H

#include <vector>
#include <functional>
#include <memory>
#include <atomic>

#include "iptux-core/ProgramData.h"
#include "iptux-core/Event.h"

namespace iptux {

using EventCallback = std::function<void(std::shared_ptr<const Event>)>;

class CoreThread {
 public:
  explicit CoreThread(std::shared_ptr<ProgramData> data);
  virtual ~CoreThread();

  virtual void start();
  virtual void stop();

  int getUdpSock() const;

  std::shared_ptr<ProgramData> getProgramData();
  bool BlacklistContainItem(in_addr_t ipv4) const;

  /**
   * @brief add ipaddress to block list
   *
   * @param ipv4 the ip address
   */
  void AddBlockIp(in_addr_t ipv4);

  /**
   * @brief whether the ipv4 address is blocked?
   *
   * @param ipv4: address
   * @return true if blocked
   * @return false if not blocked
   */
  bool IsBlocked(in_addr_t ipv4) const;

  void Lock();
  void Unlock();

  const std::vector<std::shared_ptr<PalInfo>>& GetPalList();
  virtual void ClearAllPalFromList();
  [[deprecated]] const PalInfo *GetPalFromList(PalKey palKey) const;
  [[deprecated]] PalInfo *GetPalFromList(PalKey palKey);

  CPPalInfo GetPal(PalKey palKey) const;
  PPalInfo GetPal(PalKey palKey);
  CPPalInfo GetPal(const std::string& ipv4) const;
  PPalInfo GetPal(const std::string& ipv4);

  virtual void DelPalFromList(PalKey palKey);
  virtual void UpdatePalToList(PalKey palKey);

  [[deprecated]]
  virtual void AttachPalToList(PalInfo *pal);
  virtual void AttachPalToList(PPalInfo pal);

  void registerCallback(const EventCallback &callback);
  void sendFeatureData(PPalInfo pal);
  void emitSomeoneExit(const PalKey& palKey);
  void emitNewPalOnline(PPalInfo palInfo);
  void emitNewPalOnline(const PalKey& palKey);
  void EmitIconUpdate(const PalKey& palKey);
  void emitEvent(std::shared_ptr<const Event> event);

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
  bool SendMessage(PPalInfo pal, const std::string& message);
  bool SendMessage(PPalInfo pal, const ChipData& chipData);
  bool SendMsgPara(const MsgPara& msgPara);
  void AsyncSendMsgPara(MsgPara&& msgPara);

  bool SendAskShared(PPalInfo pal);

  void SendDetectPacket(const std::string& ipv4);
  void SendDetectPacket(in_addr_t ipv4);
  void SendExit(PPalInfo pal);
  void SendMyIcon(PPalInfo pal, std::istream& iss);
  void SendSharedFiles(PPalInfo pal);

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

 public:
  static void SendNotifyToAll(CoreThread *pcthrd);

 protected:
  std::shared_ptr<ProgramData> programData;
  std::shared_ptr<IptuxConfig> config;
  int tcpSock;
  int udpSock;
  pthread_mutex_t mutex;  //锁

 private:
  std::atomic_bool started;
  std::vector<EventCallback> callbacks;

 protected:
  virtual void ClearSublayer();

 private:
  void bind_iptux_port();

 private:
  static void RecvUdpData(CoreThread *pcthrd);
  static void RecvTcpData(CoreThread *pcthrd);
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};

}

#endif //IPTUX_CORETHREAD_H
