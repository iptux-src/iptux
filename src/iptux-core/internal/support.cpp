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

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "iptux-core/internal/ipmsg.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

static void socket_enable(GSocket* sock, const char* optname, int opt) {
  GError* error = NULL;
  if (!g_socket_set_option(sock, SOL_SOCKET, opt, TRUE, &error)) {
    LOG_WARN("g_socket_set_option for %d, %s failed: %s", g_socket_get_fd(sock),
             optname, error->message);
    g_error_free(error);
  }
}

/**
 * 让套接口支持广播.
 * @param sock socket
 */
void socket_enable_broadcast(GSocket* sock) {
  socket_enable(sock, "SO_BROADCAST", SO_BROADCAST);
}

/**
 * 让套接口监听端口可重用.
 * @param sock socket
 */
void socket_enable_reuse(GSocket* sock) {
  socket_enable(sock, "SO_REUSEPORT", SO_REUSEPORT);
}

/**
 * 获取系统主机的广播地址.
 * @param sock socket
 * @return 广播地址链表
 * @note 链表数据不是指针而是实际的IP
 */
vector<string> get_sys_broadcast_addr(int sock) {
  const uint8_t amount = 5;  // 支持5个IP地址
  uint8_t count, sum;
  struct ifconf ifc;
  struct ifreq* ifr;
  struct sockaddr_in* addr;
  vector<string> res;

  res.push_back("255.255.255.255");

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
  if (res.size() == 1) {
    res.push_back("127.0.0.1");
  }
  return res;
}

}  // namespace iptux
