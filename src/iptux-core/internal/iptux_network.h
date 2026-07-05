#pragma once

#ifdef _WIN32
  #include <winsock2.h>
  #include <iphlpapi.h>
  #pragma comment(lib, "iphlpapi.lib")
  #pragma comment(lib, "ws2_32.lib")
#else
  #include <net/if.h>
  #include <ifaddrs.h>
#endif
