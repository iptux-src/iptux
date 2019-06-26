#ifndef IPTUX_CONST_H
#define IPTUX_CONST_H

#include <cstdint>

namespace iptux {

const int MAX_PREVIEWSIZE = 150;
const int MAX_BUFLEN = 1024;
const int MAX_PATHLEN = 1024;
const int MAX_ICONSIZE = 30;
const int MAX_PHOTOSIZE = 300;
const int MAX_UDPLEN = 8192;
const int MAX_SHAREDFILE = 10000;


const uint32_t IPTUX_REGULAROPT = 0x00000100UL;
const uint32_t IPTUX_SEGMENTOPT = 0x00000200UL;
const uint32_t IPTUX_GROUPOPT = 0x00000300UL;
const uint32_t IPTUX_BROADCASTOPT = 0x00000400UL;
}

#endif