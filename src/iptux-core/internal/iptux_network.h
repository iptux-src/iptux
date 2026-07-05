#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#else
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#endif

#ifndef MSG_NOSIGNAL
  #define MSG_NOSIGNAL 0
#endif

#ifdef _WIN32
typedef int socklen_t;
#endif
