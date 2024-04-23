#include "UdpServer.h"

#include <glib/gi18n.h>
#include <netinet/in.h>
#include <unistd.h>

#include "iptux-core/Exception.h"
#include "iptux-core/internal/support.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"

using namespace std;
namespace iptux {

UdpServer::UdpServer(CoreThread& coreThread) : core_thread_(coreThread) {}

unique_ptr<UdpData> UdpServer::process(in_addr ipv4,
                                       int port,
                                       const char buf[],
                                       size_t size) {
  return process(ipv4, port, buf, size, true);
}

unique_ptr<UdpData> UdpServer::process(in_addr ipv4,
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

void UdpServer::process(UdpData& udata) {
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

bool UdpServer::start() {
  if (status != UdpServerStatus::INITED) {
    LOG_ERROR("udp server status is not inited");
    return false;
  }

  int udpSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (udpSock < 0) {
    LOG_ERROR("create udp socket failed: %s", strerror(errno));
    status = UdpServerStatus::START_FAILED;
    return false;
  }

  socket_enable_reuse(udpSock);
  socket_enable_broadcast(udpSock);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(bind_port);
  addr.sin_addr = inAddrFromString(bind_ip);

  if (::bind(udpSock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    int ec = errno;
    close(udpSock);
    auto errmsg =
        stringFormat(_("Fatal Error!! Failed to bind the UDP port(%s:%d)!\n%s"),
                     bind_ip.c_str(), bind_port, strerror(ec));
    LOG_ERROR("%s", errmsg.c_str());
    throw Exception(UDP_BIND_FAILED, errmsg);
  } else {
    LOG_INFO("bind UDP port(%s:%d) success.", bind_ip.c_str(), bind_port);
  }

  status = UdpServerStatus::RUNNING;
  return true;
}

}  // namespace iptux
