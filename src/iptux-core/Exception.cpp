#include "config.h"
#include "iptux-core/Exception.h"

#include <string>

#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

const ErrorCode CREATE_TCP_SOCKET_FAILED(5001, "CREATE_TCP_SOCKET_FAILED");
const ErrorCode SOCKET_CREATE_FAILED(5002, "SOCKET_CREATE_FAILED");
const ErrorCode INVALID_FILE_ATTR(5003, "INVALID_FILE_ATTR");
const ErrorCode PAL_KEY_NOT_EXIST(5004, "PAL_KEY_NOT_EXIST");
const ErrorCode TCP_BIND_FAILED(5005, "TCP_BIND_FAILED");
const ErrorCode UDP_BIND_FAILED(5006, "UDP_BIND_FAILED");

}
