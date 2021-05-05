#ifndef IPTUX_CONNECT_SERVICE_H
#define IPTUX_CONNECT_SERVICE_H

#include "iptux-core/Models.h"
#include <string>

namespace iptux {

class ConnectService {
 public:
  virtual ~ConnectService() = default;

  virtual void start() = 0;
  virtual void stop() = 0;

  virtual bool sendMessage(const PalKey& pal, const std::string& msg) = 0;
};

}  // namespace iptux

#endif
