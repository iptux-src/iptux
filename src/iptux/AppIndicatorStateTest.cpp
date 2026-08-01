// SPDX-License-Identifier: GPL-2.0-or-later

#include "AppIndicatorState.h"

#include "gtest/gtest.h"

using namespace iptux;

TEST(AppIndicatorState, ShouldBlinkOnlyWhenBlinkModeAndUnreadPositive) {
  EXPECT_FALSE(ShouldBlink(STATUS_ICON_MODE_NONE, 1));
  EXPECT_FALSE(ShouldBlink(STATUS_ICON_MODE_NORMAL, 1));
  EXPECT_FALSE(ShouldBlink(STATUS_ICON_MODE_BLINKING, 0));
  EXPECT_TRUE(ShouldBlink(STATUS_ICON_MODE_BLINKING, 1));
}

TEST(AppIndicatorState, ResolveNormalAndAttentionIcons) {
  EXPECT_EQ(ResolveIconState(STATUS_ICON_MODE_NONE, 9, true),
            AppIndicatorIconState::kNormal);
  EXPECT_EQ(ResolveIconState(STATUS_ICON_MODE_NORMAL, 0, true),
            AppIndicatorIconState::kNormal);
  EXPECT_EQ(ResolveIconState(STATUS_ICON_MODE_NORMAL, 1, false),
            AppIndicatorIconState::kAttention);
}

TEST(AppIndicatorState, ResolveBlinkingIcons) {
  EXPECT_EQ(ResolveIconState(STATUS_ICON_MODE_BLINKING, 0, false),
            AppIndicatorIconState::kNormal);
  EXPECT_EQ(ResolveIconState(STATUS_ICON_MODE_BLINKING, 2, false),
            AppIndicatorIconState::kNormal);
  EXPECT_EQ(ResolveIconState(STATUS_ICON_MODE_BLINKING, 2, true),
            AppIndicatorIconState::kReverse);
}
