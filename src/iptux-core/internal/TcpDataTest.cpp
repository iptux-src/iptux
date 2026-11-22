// SPDX-License-Identifier: GPL-2.0-or-later

#include "gtest/gtest.h"

#define private public
#include "TcpData.h"
#undef private

#include <cstring>

using namespace iptux;

// A short or NUL-containing TCP request can have fewer than five separators.
// RequestData() must reject it before passing a NULL pointer to the reader.
TEST(TcpData, tcpRequestDataShortHeader) {
  TcpData tcpData;
  std::strcpy(tcpData.buf, "1:2:3:4");
  tcpData.size = std::strlen(tcpData.buf);

  tcpData.RequestData(FileAttr::REGULAR);

  const char nulHeader[] = "1:2\0:3:4:5:";
  std::memcpy(tcpData.buf, nulHeader, sizeof(nulHeader));
  tcpData.size = sizeof(nulHeader);
  tcpData.RequestData(FileAttr::DIRECTORY);
}
