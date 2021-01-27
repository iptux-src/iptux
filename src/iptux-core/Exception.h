#ifndef IPTUX_CORE_EXCEPTION_H
#define IPTUX_CORE_EXCEPTION_H

#include <stdexcept>

#include "iptux-utils/Exception.h"

namespace iptux {

extern const ErrorCode CREATE_TCP_SOCKET_FAILED;
extern const ErrorCode INVALID_FILE_ATTR;
extern const ErrorCode PAL_KEY_NOT_EXIST;
extern const ErrorCode SOCKET_CREATE_FAILED;
extern const ErrorCode TCP_BIND_FAILED;
extern const ErrorCode UDP_BIND_FAILED;

}

#endif //IPTUX_EXCEPTION_H
