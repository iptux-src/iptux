#include "config.h"
#include "Exception.h"

#include <string>

using namespace std;

namespace iptux {

static string ec2str(ErrorCode ec) {
  switch(ec) {
    case ErrorCode ::INVALID_IP_ADDRESS:
      return "INVALID_IP_ADDRESS";
    default:
      string res = "UNKNOWN ERROR CODE: ";
      res += int(ec);
      return res;
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
