#include "config.h"
#include "Exception.h"

#include <string>

#include "iptux-core/utils.h"

using namespace std;

namespace iptux {

static string ec2str(ErrorCode ec) {
  switch(ec) {
    case ErrorCode::TCP_BIND_FAILED:
      return "TCP_BIND_FAILED";
    case ErrorCode::UDP_BIND_FAILED:
      return "UDP_BIND_FAILED";
    case ErrorCode::PAL_KEY_NOT_EXIST:
      return "PAL_KEY_NOT_EXIST";
    case ErrorCode ::INVALID_IP_ADDRESS:
      return "INVALID_IP_ADDRESS";
    case ErrorCode::CREATE_TCP_SOCKET_FAILED:
      return "CREATE_TCP_SOCKET_FAILED";
    default:
      return stringFormat("UNKNOWN ERROR CODE: %d", int(ec));
  }
}

Exception::Exception(ErrorCode ec)
: Exception(ec, ec2str(ec))
{
}

ErrorCode Exception::getErrorCode() const {
  return ec;
}

Exception::Exception(ErrorCode ec, const std::string& reason)
    : runtime_error(reason),
      ec(ec)
{
}
}
