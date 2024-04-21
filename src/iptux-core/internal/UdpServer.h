#ifndef IPTUX_UDP_DATA_SERVICE_H
#define IPTUX_UDP_DATA_SERVICE_H

#include "iptux-core/CoreThread.h"
#include "iptux-core/internal/UdpData.h"

namespace iptux {

class UdpServer {
 public:
  explicit UdpServer(CoreThread& coreThread);

  std::unique_ptr<UdpData> process(in_addr ipv4,
                                   int port,
                                   const char buf[],
                                   size_t size);

  std::unique_ptr<UdpData> process(in_addr ipv4,
                                   int port,
                                   const char buf[],
                                   size_t size,
                                   bool run);

  void process(UdpData& udpData);

 private:
  CoreThread& core_thread_;
};

using UdpDataService_U = std::unique_ptr<UdpServer>;

}  // namespace iptux

#endif
