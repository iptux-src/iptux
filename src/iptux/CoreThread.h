#ifndef IPTUX_CORETHREAD_H
#define IPTUX_CORETHREAD_H

#include <vector>
#include <functional>

#include "iptux/ProgramData.h"
#include "iptux/Event.h"

namespace iptux {

typedef std::function<void(Event const&)> EventCallback;

class CoreThread {
 public:
  explicit CoreThread(ProgramData &data);
  virtual ~CoreThread();

  virtual void start();
  virtual void stop();

  int getUdpSock() const;

  ProgramData& getProgramData();
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

  GSList *GetPalList();
  virtual void ClearAllPalFromList();
  const PalInfo *GetPalFromList(in_addr_t ipv4) const;
  PalInfo *GetPalFromList(in_addr_t ipv4);
  virtual void DelPalFromList(in_addr_t ipv4);
  virtual void UpdatePalToList(in_addr_t ipv4);
  virtual void AttachPalToList(PalInfo *pal);

  void registerCallback(const EventCallback &callback);
  void sendFeatureData(PalInfo *pal);
  void emitNewPalOnline(PalInfo* palInfo);
  void emitEvent(const Event& event);

  /**
   * @brief send message to pal
   *
   * @param pal
   * @param message
   */
  void SendMessage(PalInfo& pal, const std::string& message);


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

 public:
  static void SendNotifyToAll(CoreThread *pcthrd);

 protected:
  ProgramData& programData;
  IptuxConfig& config;
  int tcpSock;
  int udpSock;

 private:
  GSList *blacklist;                              //黑名单链表

 protected:
  pthread_mutex_t mutex;  //锁
  GSList *pallist;  //好友链表(成员不能被删除)

 private:
  bool started;
  pthread_t notifyToAllThread;
  std::vector<EventCallback> callbacks;

 protected:
  virtual void ClearSublayer();

 private:
  void bind_iptux_port();

 private:
  static void RecvUdpData(CoreThread *pcthrd);
  static void RecvTcpData(CoreThread *pcthrd);
};

}

#endif //IPTUX_CORETHREAD_H
