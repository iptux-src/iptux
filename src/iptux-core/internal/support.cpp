//
// C++ Implementation: support
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "config.h"
#include "support.h"

#include <algorithm>

#ifdef G_OS_WIN32
#include <winsock2.h>

#include <iphlpapi.h>
#include <ws2tcpip.h>
#else
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "iptux-core/internal/iptux_network.h"
#include "iptux-core/internal/ipmsg.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

/**
 * 让套接口监听端口可重用.
 * @param sock socket
 */
void socket_enable_reuse(int sock) {
  int len;
  char optval;

  optval = 1;
  len = sizeof(optval);
#ifndef _WIN32
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &optval, len) != 0) {
    LOG_WARN("setsockopt for SO_REUSEPORT failed: %s", strerror(errno));
  }
#else
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, len) != 0) {
    LOG_WARN("setsockopt for SO_REUSEADDR failed: %s", strerror(errno));
  }
#endif
}

/**
 * Get system broadcast addresses.
 * @param sock socket
 * @return list of broadcast addresses
 */
static vector<string> get_sys_broadcast_addr_by_fd(int sock) {
  vector<string> res;

  res.push_back("255.255.255.255");

#ifdef G_OS_WIN32
  (void)sock;
  ULONG size = 0;
  constexpr ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
                          GAA_FLAG_SKIP_DNS_SERVER;
  DWORD ret = GetAdaptersAddresses(AF_INET, flags, nullptr, nullptr, &size);
  if (ret == ERROR_BUFFER_OVERFLOW && size > 0) {
    auto* adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(g_malloc0(size));
    ret = GetAdaptersAddresses(AF_INET, flags, nullptr, adapters, &size);
    if (ret == NO_ERROR) {
      for (IP_ADAPTER_ADDRESSES* adapter = adapters; adapter != nullptr;
           adapter = adapter->Next) {
        if (adapter->OperStatus != IfOperStatusUp) {
          continue;
        }
        for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress;
             unicast != nullptr; unicast = unicast->Next) {
          if (unicast->Address.lpSockaddr == nullptr ||
              unicast->Address.lpSockaddr->sa_family != AF_INET) {
            continue;
          }
          ULONG prefix = unicast->OnLinkPrefixLength;
          if (prefix > 32) {
            continue;
          }
          auto* addr =
              reinterpret_cast<sockaddr_in*>(unicast->Address.lpSockaddr);
          uint32_t ipv4 = ntohl(addr->sin_addr.s_addr);
          uint32_t mask = prefix == 0 ? 0 : (0xFFFFFFFFu << (32 - prefix));
          in_addr broadcast{};
          broadcast.s_addr = htonl((ipv4 & mask) | ~mask);
          auto broadcastAddr = inAddrToString(broadcast);
          if (find(res.begin(), res.end(), broadcastAddr) == res.end()) {
            res.push_back(broadcastAddr);
          }
        }
      }
    }
    g_free(adapters);
  }
#else
  const uint8_t amount = 5;
  uint8_t count, sum;
  struct ifconf ifc;
  struct ifreq* ifr;
  struct sockaddr_in* addr;
  ifc.ifc_len = amount * sizeof(struct ifreq);
  ifc.ifc_buf = (caddr_t)g_malloc(ifc.ifc_len);
  if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
    g_free(ifc.ifc_buf);
    return res;
  }

  sum = ifc.ifc_len / sizeof(struct ifreq);
  count = 0;
  while (count < sum) {
    ifr = ifc.ifc_req + count;
    count++;

    if (ioctl(sock, SIOCGIFFLAGS, ifr) == -1 ||
        !(ifr->ifr_flags & IFF_BROADCAST) ||
        ioctl(sock, SIOCGIFBRDADDR, ifr) == -1)
      continue;
    addr = (struct sockaddr_in*)&ifr->ifr_broadaddr;
    res.push_back(inAddrToString(addr->sin_addr));
  }
  g_free(ifc.ifc_buf);
#endif
  if (res.size() == 1) {
    res.push_back("127.0.0.1");
  }
  return res;
}

vector<string> get_sys_broadcast_addr(GSocket* sock) {
  return get_sys_broadcast_addr_by_fd(g_socket_get_fd(sock));
}

}  // namespace iptux
