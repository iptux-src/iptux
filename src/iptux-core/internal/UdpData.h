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

#include "iptux-core/CoreThread.h"
#include "iptux-core/IptuxConfig.h"
#include "iptux-core/Models.h"
#include "iptux-core/internal/CommandMode.h"
#include "iptux-core/internal/ipmsg.h"

namespace iptux {

class UdpData {
 public:
  UdpData(CoreThread& coreThread, in_addr ipv4, const char buf[], size_t size);
  UdpData(const std::string& buf, const std::string& ipv4String);
  ~UdpData();

  in_addr getIpv4() const { return ipv4; }
  std::string getIpv4String() const;

  uint32_t getCommandNo() const;
  CommandMode getCommandMode() const;

 public:
  std::shared_ptr<PalInfo> CreatePalInfo();

 public:
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

 private:
  void UpdatePalInfo(PalInfo* pal);

  void InsertMessage(PPalInfo pal, GroupBelongType btype, const char* msg);
  void ConvertEncode(const std::string& enc);
  void ConvertEncode(const char* enc);
  std::string GetPalGroup();
  std::string GetPalIcon();
  char* GetPalEncode();
  char* RecvPalIcon();
  PPalInfo AssertPalOnline();
  void RecvPalFile();

 private:
  CoreThread& coreThread;
  in_addr ipv4;          // 数据来自
  size_t size;           // 缓冲区数据有效长度
  char buf[MAX_UDPLEN];  // 数据缓冲区
  char* encode;          // 原数据编码(NULL意味着utf8)

 private:
  static void ThreadAskSharedFile(CoreThread* coreThread, PPalInfo pal);
};

}  // namespace iptux

#endif
