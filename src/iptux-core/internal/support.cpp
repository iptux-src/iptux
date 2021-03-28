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

namespace iptux {

/**
 * 让套接口支持广播.
 * @param sock socket
 */
void socket_enable_broadcast(int sock) {
  socklen_t len;
  int optval;

  optval = 1;
  len = sizeof(optval);
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, len) != 0) {
    LOG_WARN("setsockopt for SO_BROADCAST failed: %s", strerror(errno));
  }
}

/**
 * 让套接口监听端口可重用.
 * @param sock socket
 */
void socket_enable_reuse(int sock) {
  socklen_t len;
  int optval;

  optval = 1;
  len = sizeof(optval);
#ifndef __CYGWIN__
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
 * 获取系统主机的广播地址.
 * @param sock socket
 * @return 广播地址链表
 * @note 链表数据不是指针而是实际的IP
 */
GSList* get_sys_broadcast_addr(int sock) {
  const uint8_t amount = 5;  //支持5个IP地址
  uint8_t count, sum;
  struct ifconf ifc;
  struct ifreq* ifr;
  struct sockaddr_in* addr;
  GSList* list;

  list = g_slist_append(NULL, GUINT_TO_POINTER(inet_addr("255.255.255.255")));
  ifc.ifc_len = amount * sizeof(struct ifreq);
  ifc.ifc_buf = (caddr_t)g_malloc(ifc.ifc_len);
  if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
    g_free(ifc.ifc_buf);
    return list;
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
    list = g_slist_append(list, GUINT_TO_POINTER(addr->sin_addr.s_addr));
  }
  g_free(ifc.ifc_buf);

  return list;
}

/**
 * 获取系统主机的IP地址.
 * @param sock socket
 * @return IP地址链表
 * @note 链表数据不是指针而是实际的IP
 */
GSList* get_sys_host_addr(int sock) {
  const uint8_t amount = 5;  //支持5个IP地址
  uint8_t count, sum;
  struct ifconf ifc;
  struct ifreq* ifr;
  struct sockaddr_in* addr;
  GSList* list;

  list = NULL;
  ifc.ifc_len = amount * sizeof(struct ifreq);
  ifc.ifc_buf = (caddr_t)g_malloc(ifc.ifc_len);
  if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
    g_free(ifc.ifc_buf);
    return list;
  }

  sum = ifc.ifc_len / sizeof(struct ifreq);
  count = 0;
  while (count < sum) {
    ifr = ifc.ifc_req + count;
    count++;

    if (strncasecmp(ifr->ifr_name, "lo", 2) == 0 ||
        ioctl(sock, SIOCGIFFLAGS, ifr) == -1 || !(ifr->ifr_flags & IFF_UP) ||
        ioctl(sock, SIOCGIFADDR, ifr) == -1)
      continue;
    addr = (struct sockaddr_in*)&ifr->ifr_broadaddr;
    list = g_slist_append(list, GUINT_TO_POINTER(addr->sin_addr.s_addr));
  }
  g_free(ifc.ifc_buf);

  return list;
}

/**
 * 获取系统主机的IP地址串描述
 * @param sock socket
 * @return 描述串
 */
char* get_sys_host_addr_string(int sock) {
  char ipstr[INET_ADDRSTRLEN], *buf, *ptr;
  GSList *list, *tlist;
  uint16_t len;

  if (!(tlist = list = get_sys_host_addr(sock)))
    return NULL;

  len = g_slist_length(list) * INET_ADDRSTRLEN;
  ptr = buf = (char*)g_malloc(len);
  while (tlist) {
    inet_ntop(AF_INET, &tlist->data, ipstr, INET_ADDRSTRLEN);
    snprintf(ptr, len, "%s\n", ipstr);
    ptr += strlen(ptr);
    len -= INET_ADDRSTRLEN;
    tlist = g_slist_next(tlist);
  }
  *(ptr - 1) = '\0';  //抹除最后一个换行符
  g_slist_free(list);

  return buf;
}

}  // namespace iptux
