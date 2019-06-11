#ifndef IPTUX_EXCEPTION_H
#define IPTUX_EXCEPTION_H

#include <stdexcept>

namespace iptux {

enum class ErrorCode {
  TCP_BIND_FAILED,
  UDP_BIND_FAILED,
  INVALID_IP_ADDRESS,
  PAL_KEY_NOT_EXIST,
  CREATE_TCP_SOCKET_FAILED,
};

class Exception : public std::runtime_error {
 public:
  explicit Exception(ErrorCode ec);
  Exception(ErrorCode ec, const std::string& reason);
  Exception(ErrorCode ec, std::exception* causedBy);
  Exception(ErrorCode ec, const std::string& reason, std::exception* causedBy);

  ErrorCode getErrorCode() const;
 private:
  ErrorCode ec;
};

}

#endif //IPTUX_EXCEPTION_H
