//
// C++ Interface: UdpData
//
// Description:
// 处理接收到的UDP数据
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_UDPDATA_H
#define IPTUX_UDPDATA_H

#include <string>

#include "iptux-core/IptuxConfig.h"
#include "iptux-core/internal/ipmsg.h"
#include "iptux-core/Models.h"
#include "iptux-core/CoreThread.h"

namespace iptux {

class UdpData {
 public:
  explicit UdpData(CoreThread& coreThread);
  ~UdpData();

  static std::unique_ptr<UdpData> UdpDataEntry(CoreThread& coreThread,
                           in_addr ipv4,
                           int port,
                           const char buf[],
                           size_t size);
  static std::unique_ptr<UdpData> UdpDataEntry(CoreThread& coreThread,
                           in_addr ipv4,
                           int port,
                           const char buf[],
                           size_t size,
                           bool run /** if run is false, don't do DispatchUdpData */
                           );

 // for test
 public:
  std::shared_ptr<PalInfo> CreatePalInfo();


 private:
  void DispatchUdpData();

  void SomeoneLost();
  void SomeoneEntry();
  void SomeoneExit();
  void SomeoneAnsEntry();
  void SomeoneAbsence();
  void SomeoneSendmsg();
  void SomeoneRecvmsg();
  void SomeoneAskShared();
  void SomeoneSendIcon();
  void SomeoneSendSign();
  void SomeoneBcstmsg();

  void UpdatePalInfo(PalInfo *pal);

  void InsertMessage(PPalInfo pal, GroupBelongType btype, const char *msg);
  void ConvertEncode(const std::string &enc);
  void ConvertEncode(const char *enc);
  std::string GetPalGroup();
  char *GetPalIcon();
  char *GetPalEncode();
  char *RecvPalIcon();
  PPalInfo AssertPalOnline();
  void RecvPalFile();

 private:
  CoreThread& coreThread;
  in_addr ipv4;        //数据来自
  size_t size;           //缓冲区数据有效长度
  char buf[MAX_UDPLEN];  //数据缓冲区
  char *encode;          //原数据编码(NULL意味着utf8)

 private:
  static void ThreadAskSharedFile(CoreThread* coreThread, PPalInfo pal);
};

}  // namespace iptux

#endif
