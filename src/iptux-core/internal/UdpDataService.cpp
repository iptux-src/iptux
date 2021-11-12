#include "UdpDataService.h"

#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"

using namespace std;
namespace iptux {

UdpDataService::UdpDataService(CoreThread& coreThread)
    : core_thread_(coreThread) {}

unique_ptr<UdpData> UdpDataService::process(in_addr ipv4,
                                            int port,
                                            const char buf[],
                                            size_t size) {
  return process(ipv4, port, buf, size, true);
}

unique_ptr<UdpData> UdpDataService::process(in_addr ipv4,
                                            int port,
                                            const char buf[],
                                            size_t size,
                                            bool run) {
  if (Log::IsDebugEnabled()) {
    LOG_DEBUG("received udp message from %s:%d, size %zu\n%s",
              inAddrToString(ipv4).c_str(), port, size,
              stringDumpAsCString(string(buf, size)).c_str());
  } else {
    LOG_INFO("received udp message from %s:%d, size %zu",
             inAddrToString(ipv4).c_str(), port, size);
  }
  auto udata = make_unique<UdpData>(core_thread_, ipv4, buf, size);

  if (run) {
    udata->DispatchUdpData();
  }
  return udata;
}

}  // namespace iptux
