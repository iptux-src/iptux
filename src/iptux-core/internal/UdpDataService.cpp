#include "UdpDataService.h"

using namespace iptux;

UdpDataService::UdpDataService(CoreThread& coreThread)
    : core_thread_(coreThread) {}

std::unique_ptr<UdpData> UdpDataService::process(in_addr ipv4,
                                                 int port,
                                                 const char buf[],
                                                 size_t size) {
  return UdpData::UdpDataEntry(core_thread_, ipv4, port, buf, size, true);
}
