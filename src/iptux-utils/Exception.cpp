#include "config.h"
#include "Exception.h"

#include <string>

#include "iptux-utils/utils.h"

using namespace std;

namespace iptux {

const ErrorCode INVALID_IP_ADDRESS(4001, "INVALID_IP_ADDRESS");

Exception::Exception(const ErrorCode& ec)
: Exception(ec, ec.getMessage())
{
}

const ErrorCode& Exception::getErrorCode() const {
  return ec;
}

Exception::Exception(const ErrorCode& ec, const std::string& reason)
    : runtime_error(reason),
      ec(ec)
{
}
}
