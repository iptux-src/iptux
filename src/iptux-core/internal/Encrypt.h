// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <string>

namespace iptux {

class Encrypt {
 public:
  static std::string genRsaPrivPem(int bits);
};

}  // namespace iptux