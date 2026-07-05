#pragma once

#ifdef _WIN32
  #include <winsock2.h>
  #include <iphlpapi.h>
#else
  #include <net/if.h>
  #include <ifaddrs.h>
#endif

#ifndef MSG_NOSIGNAL
  #define MSG_NOSIGNAL 0
#endif
