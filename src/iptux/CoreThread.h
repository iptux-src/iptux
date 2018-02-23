#ifndef IPTUX_CORETHREAD_H
#define IPTUX_CORETHREAD_H

#include "iptux/ProgramDataCore.h"

namespace iptux {

class CoreThread {
 public:
  explicit CoreThread(ProgramDataCore &data);
  virtual ~CoreThread();

  virtual void start();
  virtual void stop();

  bool getDebug() const;
  void setDebug(bool debug);

  int getUdpSock() const;

  ProgramDataCore& getProgramData();
  bool BlacklistContainItem(in_addr_t ipv4) const;

  void Lock();
  void Unlock();

  GSList *GetPalList();
  virtual void ClearAllPalFromList();
  const PalInfo *GetPalFromList(in_addr_t ipv4) const;
  PalInfo *GetPalFromList(in_addr_t ipv4);
  virtual void DelPalFromList(in_addr_t ipv4);
  virtual void UpdatePalToList(in_addr_t ipv4);
  virtual void AttachPalToList(PalInfo *pal);
 public:
  static void SendNotifyToAll(CoreThread *pcthrd);
 protected:
  ProgramDataCore& programData;
  IptuxConfig& config;
  int tcpSock;
  int udpSock;
  bool debug;
  GSList *blacklist;                              //黑名单链表
  pthread_mutex_t mutex;  //锁
  GSList *pallist;  //好友链表(成员不能被删除)
 private:
  bool started;
  pthread_t notifyToAllThread;
 protected:
  virtual void ClearSublayer();
 private:
  void bind_iptux_port();
 private:
  static void RecvUdpData(CoreThread *pcthrd);
  static void RecvTcpData(CoreThread *pcthrd);
  static gboolean WatchCoreStatus(CoreThread *pcthrd);
};

}

#endif //IPTUX_CORETHREAD_H
