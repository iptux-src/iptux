//
// C++ Interface: Command
//
// Description:
// 创建命令并发送
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IPTUX_COMMAND_H
#define IPTUX_COMMAND_H

#include <string>
#include <istream>

#include "iptux/ipmsg.h"
#include "iptux-core/Models.h"
#include "iptux-core/CoreThread.h"

namespace iptux {

class Command {
 public:
  explicit Command(CoreThread& coreThread);
  ~Command();

  Command(const Command&) = delete;
  Command& operator=(const Command&) = delete;

  /// Const Pointer to PalInfo
  using CPPalInfo = std::shared_ptr<const PalInfo>;

  void BroadCast(int sock);
  void DialUp(int sock);
  void SendAnsentry(int sock, CPPalInfo pal);
  void SendExit(int sock, CPPalInfo pal);
  void SendAbsence(int sock, CPPalInfo pal);
  void SendDetectPacket(int sock, in_addr_t ipv4);
  void SendMessage(int sock, CPPalInfo pal, const char *msg);
  void SendReply(int sock, CPPalInfo pal, uint32_t packetno);
  void SendReply(int sock, const PalKey& pal, uint32_t packetno);
  void SendGroupMsg(int sock, CPPalInfo pal, const char *msg);
  void SendUnitMsg(int sock, CPPalInfo pal, uint32_t opttype, const char *msg);

  bool SendAskData(int sock, CPPalInfo pal, uint32_t packetno, uint32_t fileid,
                   int64_t offset);
  bool SendAskData(int sock, const PalKey& pal, uint32_t packetno, uint32_t fileid,
                   int64_t offset);
  bool SendAskFiles(int sock, CPPalInfo pal, uint32_t packetno, uint32_t fileid);
  bool SendAskFiles(int sock, const PalKey& pal, uint32_t packetno, uint32_t fileid);
  void SendAskShared(int sock, CPPalInfo pal, uint32_t opttype,
                     const char *attach);
  void SendAskShared(int sock, const PalKey& pal, uint32_t opttype,
                     const char *attach);
  void SendFileInfo(int sock, CPPalInfo pal, uint32_t opttype,
                    const char *extra);
  void SendFileInfo(int sock, const PalKey& pal, uint32_t opttype,
                    const char *extra);
  void SendMyIcon(int sock, CPPalInfo pal, std::istream& iss);
  void SendMySign(int sock, CPPalInfo pal);
  void SendSublayer(int sock, CPPalInfo pal, uint32_t opttype, const char *path);

 private:
  void FeedbackError(CPPalInfo pal, GroupBelongType btype, const char *error);
  void SendSublayerData(int sock, int fd);
  void ConvertEncode(const std::string &encode);
  void CreateCommand(uint32_t command, const char *attach);
  void CreateIpmsgExtra(const char *extra, const char *encode);
  void CreateIptuxExtra(const std::string &encode);
  void CreateIconExtra(std::istream& iss);

 private:
  CoreThread& coreThread;
  size_t size;              //当前已使用缓冲区的长度
  char buf[MAX_UDPLEN];     //数据缓冲区
  static uint32_t packetn;  //包编号

 public:
  inline uint32_t &Packetn() { return packetn; }
};

}  // namespace iptux

#endif
