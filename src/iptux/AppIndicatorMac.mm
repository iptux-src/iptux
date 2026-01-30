// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.h"
#include "AppIndicator.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#import <Cocoa/Cocoa.h>

// Objective-C helper class for NSStatusItem callbacks.
// Must be at global scope (Objective-C declarations cannot appear inside a C++
// namespace).
@interface IptuxStatusItemHelper : NSObject {
  GActionGroup* actionGroup_;
  iptux::IptuxAppIndicator* owner_;
}
- (instancetype)initWithActionGroup:(GActionGroup*)actionGroup
                              owner:(iptux::IptuxAppIndicator*)owner;
- (void)openMainWindow:(id)sender;
- (void)openPreferences:(id)sender;
- (void)quit:(id)sender;
- (void)statusItemClicked:(id)sender;
@end

@implementation IptuxStatusItemHelper
- (instancetype)initWithActionGroup:(GActionGroup*)actionGroup
                              owner:(iptux::IptuxAppIndicator*)owner {
  self = [super init];
  if (self) {
    actionGroup_ = actionGroup;
    owner_ = owner;
  }
  return self;
}

- (void)openMainWindow:(id)sender {
  (void)sender;
  g_action_group_activate_action(actionGroup_, "open_main_window", NULL);
}

- (void)openPreferences:(id)sender {
  (void)sender;
  g_action_group_activate_action(actionGroup_, "preferences", NULL);
}

- (void)quit:(id)sender {
  (void)sender;
  g_action_group_activate_action(actionGroup_, "quit", NULL);
}

- (void)statusItemClicked:(id)sender {
  (void)sender;
  owner_->sigActivateMainWindow.emit();
}
@end

static NSImage* pixbufToNSImage(GdkPixbuf* pixbuf) {
  int width = gdk_pixbuf_get_width(pixbuf);
  int height = gdk_pixbuf_get_height(pixbuf);
  int stride = gdk_pixbuf_get_rowstride(pixbuf);
  gboolean hasAlpha = gdk_pixbuf_get_has_alpha(pixbuf);
  const guchar* pixels = gdk_pixbuf_get_pixels(pixbuf);

  int bitsPerSample = 8;
  int channels = hasAlpha ? 4 : 3;

  NSBitmapImageRep* rep = [[NSBitmapImageRep alloc]
      initWithBitmapDataPlanes:NULL
                    pixelsWide:width
                    pixelsHigh:height
                 bitsPerSample:bitsPerSample
               samplesPerPixel:channels
                      hasAlpha:hasAlpha
                      isPlanar:NO
                colorSpaceName:NSDeviceRGBColorSpace
                   bytesPerRow:stride
                  bitsPerPixel:bitsPerSample * channels];

  memcpy([rep bitmapData], pixels, height * stride);

  NSImage* image = [[NSImage alloc] initWithSize:NSMakeSize(width, height)];
  [image addRepresentation:rep];
  [rep release];

  return [image autorelease];
}

static NSImage* loadIcon(const char* iconName, int size) {
  auto theme = gtk_icon_theme_get_default();
  GError* error = nullptr;
  auto pixbuf = gtk_icon_theme_load_icon(theme, iconName, size,
                                         GtkIconLookupFlags(0), &error);
  if (!pixbuf) {
    g_warning("Couldn't load icon %s: %s", iconName, error->message);
    g_error_free(error);
    return nil;
  }
  NSImage* image = pixbufToNSImage(pixbuf);
  [image retain];
  g_object_unref(pixbuf);

  // Resize for menu bar (typically 18x18 points)
  [image setSize:NSMakeSize(18, 18)];
  return image;
}

namespace iptux {

class IptuxAppIndicatorPrivate {
 public:
  IptuxAppIndicatorPrivate() {}
  ~IptuxAppIndicatorPrivate() {
    if (blinkTimerId) {
      g_source_remove(blinkTimerId);
    }
    if (statusItem) {
      [[NSStatusBar systemStatusBar] removeStatusItem:statusItem];
      [statusItem release];
    }
    if (helper) {
      [helper release];
    }
    if (normalIcon) {
      [normalIcon release];
    }
    if (attentionIcon) {
      [attentionIcon release];
    }
    if (reverseIcon) {
      [reverseIcon release];
    }
  }

  NSStatusItem* statusItem = nil;
  IptuxStatusItemHelper* helper = nil;
  NSImage* normalIcon = nil;
  NSImage* attentionIcon = nil;
  NSImage* reverseIcon = nil;
  StatusIconMode mode = STATUS_ICON_MODE_NORMAL;
  int unreadCount = 0;
  guint blinkTimerId = 0;
  bool blinkState = false;
};

IptuxAppIndicator::IptuxAppIndicator(GActionGroup* action_group) {
  this->priv = std::make_shared<IptuxAppIndicatorPrivate>();

  priv->helper = [[IptuxStatusItemHelper alloc] initWithActionGroup:action_group
                                                              owner:this];

  // Load icons
  priv->normalIcon = loadIcon("iptux-icon", 64);
  priv->attentionIcon = loadIcon("iptux-attention", 64);
  priv->reverseIcon = loadIcon("iptux-icon-reverse", 64);

  // Create status item
  priv->statusItem =
      [[[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength] retain];

  if (priv->normalIcon) {
    priv->statusItem.button.image = priv->normalIcon;
  }
  priv->statusItem.button.toolTip = @"Iptux";

  // Build the menu
  NSMenu* menu = [[NSMenu alloc] init];

  NSMenuItem* openItem =
      [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:_("Open Iptux")]
                                 action:@selector(openMainWindow:)
                          keyEquivalent:@""];
  openItem.target = priv->helper;
  [menu addItem:openItem];
  [openItem release];

  [menu addItem:[NSMenuItem separatorItem]];

  NSMenuItem* prefsItem =
      [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:_("Preferences")]
                                 action:@selector(openPreferences:)
                          keyEquivalent:@""];
  prefsItem.target = priv->helper;
  [menu addItem:prefsItem];
  [prefsItem release];

  NSMenuItem* quitItem =
      [[NSMenuItem alloc] initWithTitle:[NSString stringWithUTF8String:_("Quit")]
                                 action:@selector(quit:)
                          keyEquivalent:@""];
  quitItem.target = priv->helper;
  [menu addItem:quitItem];
  [quitItem release];

  priv->statusItem.menu = menu;
  [menu release];
}

static gboolean blinkTimerCallback(gpointer data) {
  auto priv = static_cast<IptuxAppIndicatorPrivate*>(data);
  if (!priv->statusItem) return G_SOURCE_REMOVE;
  priv->blinkState = !priv->blinkState;
  if (priv->blinkState && priv->reverseIcon) {
    priv->statusItem.button.image = priv->reverseIcon;
  } else if (priv->normalIcon) {
    priv->statusItem.button.image = priv->normalIcon;
  }
  return G_SOURCE_CONTINUE;
}

static void startBlinkTimer(IptuxAppIndicatorPrivate* priv) {
  if (priv->blinkTimerId) return;
  priv->blinkState = false;
  priv->blinkTimerId = g_timeout_add(500, blinkTimerCallback, priv);
}

static void stopBlinkTimer(IptuxAppIndicatorPrivate* priv) {
  if (priv->blinkTimerId) {
    g_source_remove(priv->blinkTimerId);
    priv->blinkTimerId = 0;
  }
  priv->blinkState = false;
}

void IptuxAppIndicator::SetUnreadCount(int count) {
  priv->unreadCount = count;
  if (!priv->statusItem || priv->mode == STATUS_ICON_MODE_NONE) return;

  if (priv->mode == STATUS_ICON_MODE_BLINKING) {
    if (count > 0) {
      startBlinkTimer(priv.get());
    } else {
      stopBlinkTimer(priv.get());
      if (priv->normalIcon) {
        priv->statusItem.button.image = priv->normalIcon;
      }
    }
    return;
  }

  if (count > 0 && priv->attentionIcon) {
    priv->statusItem.button.image = priv->attentionIcon;
  } else if (priv->normalIcon) {
    priv->statusItem.button.image = priv->normalIcon;
  }
}

void IptuxAppIndicator::SetMode(StatusIconMode mode) {
  StatusIconMode oldMode = priv->mode;
  priv->mode = mode;
  if (!priv->statusItem) return;
  priv->statusItem.visible = (mode != STATUS_ICON_MODE_NONE);

  if (oldMode == STATUS_ICON_MODE_BLINKING) {
    stopBlinkTimer(priv.get());
    if (priv->normalIcon) {
      priv->statusItem.button.image = priv->normalIcon;
    }
  }

  if (mode != STATUS_ICON_MODE_NONE) {
    SetUnreadCount(priv->unreadCount);
  }
}

void IptuxAppIndicator::StopBlinking() {
  stopBlinkTimer(priv.get());
  if (!priv->statusItem || priv->mode == STATUS_ICON_MODE_NONE) return;
  if (priv->normalIcon) {
    priv->statusItem.button.image = priv->normalIcon;
  }
}

}  // namespace iptux
