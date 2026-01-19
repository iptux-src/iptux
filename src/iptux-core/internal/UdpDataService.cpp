#include "UdpDataService.h"

#include "gio/gio.h"
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

unique_ptr<UdpData> UdpDataService::process(GSocketAddress* peer,
                                            const char buf[],
                                            size_t size) {
  GInetSocketAddress* isa = G_INET_SOCKET_ADDRESS(peer);
  guint16 port = g_inet_socket_address_get_port(isa);
  GInetAddress* ia = g_inet_socket_address_get_address(isa);
  char* ip = g_inet_address_to_string(ia);

  // TODO: too many conversions, optimize it
  in_addr ipv4 = inAddrFromString(ip);
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
    process(*udata);
  }
  return udata;
}

void UdpDataService::process(UdpData& udata) {
  /* 如果开启了黑名单处理功能，且此地址正好被列入了黑名单 */
  if (core_thread_.IsBlocked(udata.getIpv4())) {
    LOG_INFO("address is blocked: %s", udata.getIpv4String().c_str());
    return;
  }

  /* 决定消息去向 */
  auto commandMode = udata.getCommandMode();
  LOG_INFO("command NO.: [0x%x] %s", udata.getCommandNo(),
           commandMode.toString().c_str());
  switch (commandMode.getMode()) {
    case IPMSG_BR_ENTRY:
      udata.SomeoneEntry();
      break;
    case IPMSG_BR_EXIT:
      udata.SomeoneExit();
      break;
    case IPMSG_ANSENTRY:
      udata.SomeoneAnsEntry();
      break;
    case IPMSG_BR_ABSENCE:
      udata.SomeoneAbsence();
      break;
    case IPMSG_SENDMSG:
      udata.SomeoneSendmsg();
      break;
    case IPMSG_RECVMSG:
      udata.SomeoneRecvmsg();
      break;
    case IPTUX_ASKSHARED:
      udata.SomeoneAskShared();
      break;
    case IPTUX_SENDICON:
      udata.SomeoneSendIcon();
      break;
    case IPTUX_SEND_SIGN:
      udata.SomeoneSendSign();
      break;
    case IPTUX_SENDMSG:
      udata.SomeoneBcstmsg();
      break;
    default:
      LOG_WARN("unknown command mode: 0x%x", commandMode.getMode());
      break;
  }
}

}  // namespace iptux
