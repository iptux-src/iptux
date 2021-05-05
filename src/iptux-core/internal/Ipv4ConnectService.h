#ifndef IPTUX_IPV4_CONNECT_SERVICE_H
#define IPTUX_IPV4_CONNECT_SERVICE_H

#include "iptux-core/CoreThread.h"
#include "iptux-core/internal/ConnectService.h"
#include <future>
#include <string>

namespace iptux {

class Ipv4ConnectService : public ConnectService {
 public:
  Ipv4ConnectService(CoreThread* coreThread, const std::string& ip, int port);
  void start() override;
  void stop() override;
  bool sendMessage(const PalKey& pal, const std::string& msg) override;

 private:
  void recvUdpData();
  void recvTcpData();

 private:
  CoreThread* coreThread;
  std::string ip;
  int port;

  bool started;
  int tcpSock = -1;
  int udpSock = -1;
  std::future<void> udpFuture;
  std::future<void> tcpFuture;
};

}  // namespace iptux

#endif
