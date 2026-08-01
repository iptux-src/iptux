// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "iptux-core/StatusIconMode.h"

namespace iptux {

enum class AppIndicatorIconState {
  kNormal,
  kAttention,
  kReverse,
};

inline bool ShouldBlink(StatusIconMode mode, int unread_count) {
  return mode == STATUS_ICON_MODE_BLINKING && unread_count > 0;
}

inline AppIndicatorIconState ResolveIconState(StatusIconMode mode,
                                              int unread_count,
                                              bool blink_state) {
  if (mode == STATUS_ICON_MODE_BLINKING) {
    if (unread_count > 0 && blink_state) {
      return AppIndicatorIconState::kReverse;
    }
    return AppIndicatorIconState::kNormal;
  }
  if (mode == STATUS_ICON_MODE_NORMAL && unread_count > 0) {
    return AppIndicatorIconState::kAttention;
  }
  return AppIndicatorIconState::kNormal;
}

}  // namespace iptux
