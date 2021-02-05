#ifndef IPTUX_UTILS_EXCEPTION_H
#define IPTUX_UTILS_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace iptux {

class ErrorCode {
 private:
  int code;
  std::string message;
 public:
  ErrorCode(int code, const std::string& message)
  : code(code), message(message)
  {}
  ErrorCode(const ErrorCode&) = delete;
  ErrorCode& operator=(const ErrorCode&) = delete;

  int getCode() const;
  const std::string& getMessage() const {
    return message;
  }

  bool operator==(const ErrorCode& rhs) const {
    return this->code == rhs.code;
  }
};

extern const ErrorCode INVALID_IP_ADDRESS;

class Exception : public std::runtime_error {
 public:
  explicit Exception(const ErrorCode& ec);
  Exception(const ErrorCode& ec, const std::string& reason);
  Exception(const ErrorCode& ec, std::exception* causedBy);
  Exception(const ErrorCode& ec, const std::string& reason, std::exception* causedBy);

  const ErrorCode& getErrorCode() const;
 private:
  const ErrorCode& ec;
};

}

#endif //IPTUX_EXCEPTION_H
