#include "config.h"
#include "Ipv4ConnectService.h"

#include "iptux-core/Exception.h"
#include "iptux-core/internal/TcpData.h"
#include "iptux-core/internal/UdpData.h"
#include "iptux-core/internal/support.h"
#include "iptux-utils/output.h"
#include "iptux-utils/utils.h"
#include <cerrno>
#include <cstdio>
#include <glib/gi18n.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

namespace iptux {

Ipv4ConnectService::Ipv4ConnectService(CoreThread* coreThread,
                                       const string& ip,
                                       int port)
    : coreThread(coreThread), ip(ip), port(port), started(false) {}

void Ipv4ConnectService::start() {
  started = true;
  tcpSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_enable_reuse(tcpSock);
  udpSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  socket_enable_reuse(udpSock);
  socket_enable_broadcast(udpSock);
  if ((tcpSock == -1) || (udpSock == -1)) {
    int ec = errno;
    auto errmsg = stringFormat(
        _("Fatal Error!! Failed to create new socket!\n%s"), strerror(ec));
    LOG_WARN("%s", errmsg.c_str());
    throw Exception(SOCKET_CREATE_FAILED, errmsg);
  }

  struct sockaddr_in addr;
  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr = inAddrFromString(ip);

  if (::bind(tcpSock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    int ec = errno;
    close(tcpSock);
    close(udpSock);
    auto errmsg =
        stringFormat(_("Fatal Error!! Failed to bind the TCP port(%s:%d)!\n%s"),
                     ip.c_str(), port, strerror(ec));
    LOG_ERROR("%s", errmsg.c_str());
    throw Exception(TCP_BIND_FAILED, errmsg);
  } else {
    LOG_INFO("bind TCP port(%s:%d) success.", ip.c_str(), port);
  }

  if (::bind(udpSock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    int ec = errno;
    close(tcpSock);
    close(udpSock);
    auto errmsg =
        stringFormat(_("Fatal Error!! Failed to bind the UDP port(%s:%d)!\n%s"),
                     ip.c_str(), port, strerror(ec));
    LOG_ERROR("%s", errmsg.c_str());
    throw Exception(UDP_BIND_FAILED, errmsg);
  } else {
    LOG_INFO("bind UDP port(%s:%d) success.", ip.c_str(), port);
  }

  udpFuture =
      async([](Ipv4ConnectService* self) { self->recvUdpData(); }, this);
  tcpFuture =
      async([](Ipv4ConnectService* self) { self->recvTcpData(); }, this);
}

void Ipv4ConnectService::stop() {
  started = false;
  udpFuture.wait();
  tcpFuture.wait();
  close(udpSock);
  close(tcpSock);
}

void Ipv4ConnectService::recvUdpData() {
  struct sockaddr_in addr;
  socklen_t len;
  ssize_t size;
  char buf[MAX_UDPLEN];

  while (started) {
    struct pollfd pfd = {udpSock, POLLIN, 0};
    int ret = poll(&pfd, 1, 10);
    if (ret == -1) {
      LOG_ERROR("poll udp socket failed: %s", strerror(errno));
      return;
    }
    if (ret == 0) {
      continue;
    }
    g_assert(ret == 1);
    len = sizeof(addr);
    if ((size = recvfrom(udpSock, buf, MAX_UDPLEN, 0, (struct sockaddr*)&addr,
                         &len)) == -1)
      continue;
    if (size != MAX_UDPLEN)
      buf[size] = '\0';
    auto port = ntohs(addr.sin_port);
    UdpData::UdpDataEntry(*coreThread, addr.sin_addr, port, buf, size);
  }
}

void Ipv4ConnectService::recvTcpData() {
  int subsock;

  listen(tcpSock, 5);
  while (started) {
    struct pollfd pfd = {tcpSock, POLLIN, 0};
    int ret = poll(&pfd, 1, 10);
    if (ret == -1) {
      LOG_ERROR("poll udp socket failed: %s", strerror(errno));
      return;
    }
    if (ret == 0) {
      continue;
    }
    g_assert(ret == 1);
    if ((subsock = accept(tcpSock, NULL, NULL)) == -1)
      continue;
    thread([](CoreThread* coreThread,
              int subsock) { TcpData::TcpDataEntry(coreThread, subsock); },
           coreThread, subsock)
        .detach();
  }
}

bool Ipv4ConnectService::sendMessage(const PalKey& pal,
                                     const std::string& msg) {
  if (Log::IsDebugEnabled()) {
    LOG_DEBUG("send udp message to %s:%d, size %d\n%s",
              inAddrToString(pal.GetIpv4()).c_str(), pal.GetPort(),
              int(msg.size()), stringDump(msg).c_str());
  } else if (Log::IsInfoEnabled()) {
    LOG_INFO("send udp message to %s:%d, size %d",
             inAddrToString(pal.GetIpv4()).c_str(), pal.GetPort(),
             int(msg.size()));
  }
  struct sockaddr_in addr;
  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(pal.GetPort());
  addr.sin_addr = pal.GetIpv4();
  return sendto(udpSock, &msg[0], msg.size(), 0, (struct sockaddr*)(&addr),
                sizeof(struct sockaddr_in)) != -1;
}

}  // namespace iptux
