// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.h"
#include "AppIndicator.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <windows.h>

#include <shellapi.h>

#include <cstdint>
#include <cwchar>
#include <string>

#include "AppIndicatorState.h"
#include "iptux-utils/output.h"

namespace iptux {

namespace {

constexpr UINT kTrayCallbackMessage = WM_APP + 101;
constexpr UINT kTrayIconId = 1;
constexpr UINT_PTR kMenuOpenId = 1001;
constexpr UINT_PTR kMenuPreferencesId = 1002;
constexpr UINT_PTR kMenuQuitId = 1003;
constexpr wchar_t kTrayWindowClass[] = L"IptuxWindowsAppIndicator";

std::wstring Utf8ToWide(const char* text) {
  if (!text) {
    return L"";
  }
  int len = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
  if (len <= 0) {
    return L"";
  }
  std::wstring result(static_cast<size_t>(len), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, text, -1, result.data(), len);
  if (!result.empty() && result.back() == L'\0') {
    result.pop_back();
  }
  return result;
}

HICON CreateHIconFromPixbuf(GdkPixbuf* pixbuf) {
  if (!pixbuf) {
    return nullptr;
  }

  const int width = gdk_pixbuf_get_width(pixbuf);
  const int height = gdk_pixbuf_get_height(pixbuf);
  if (width <= 0 || height <= 0) {
    return nullptr;
  }

  const int channels = gdk_pixbuf_get_n_channels(pixbuf);
  const int stride = gdk_pixbuf_get_rowstride(pixbuf);
  const guchar* pixels = gdk_pixbuf_get_pixels(pixbuf);
  if (!pixels || channels < 3) {
    return nullptr;
  }

  BITMAPV5HEADER bi = {};
  bi.bV5Size = sizeof(BITMAPV5HEADER);
  bi.bV5Width = width;
  bi.bV5Height = -height;
  bi.bV5Planes = 1;
  bi.bV5BitCount = 32;
  bi.bV5Compression = BI_BITFIELDS;
  bi.bV5RedMask = 0x00FF0000;
  bi.bV5GreenMask = 0x0000FF00;
  bi.bV5BlueMask = 0x000000FF;
  bi.bV5AlphaMask = 0xFF000000;

  HDC hdc = GetDC(nullptr);
  void* dibData = nullptr;
  HBITMAP colorBitmap =
      CreateDIBSection(hdc, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS,
                       &dibData, nullptr, 0);
  ReleaseDC(nullptr, hdc);
  if (!colorBitmap || !dibData) {
    if (colorBitmap) {
      DeleteObject(colorBitmap);
    }
    return nullptr;
  }

  uint8_t* out = static_cast<uint8_t*>(dibData);
  for (int y = 0; y < height; ++y) {
    const guchar* row = pixels + y * stride;
    for (int x = 0; x < width; ++x) {
      const guchar* px = row + x * channels;
      const int index = (y * width + x) * 4;
      out[index + 0] = px[2];
      out[index + 1] = px[1];
      out[index + 2] = px[0];
      out[index + 3] = channels >= 4 ? px[3] : 255;
    }
  }

  HBITMAP maskBitmap = CreateBitmap(width, height, 1, 1, nullptr);
  if (!maskBitmap) {
    DeleteObject(colorBitmap);
    return nullptr;
  }

  ICONINFO iconInfo = {};
  iconInfo.fIcon = TRUE;
  iconInfo.hbmColor = colorBitmap;
  iconInfo.hbmMask = maskBitmap;

  HICON icon = CreateIconIndirect(&iconInfo);
  DeleteObject(maskBitmap);
  DeleteObject(colorBitmap);
  return icon;
}

HICON LoadIconFromTheme(const char* icon_name, int size) {
  GtkIconTheme* theme = gtk_icon_theme_get_default();
  gtk_icon_theme_append_search_path(theme, __ICON_PATH);

  GError* error = nullptr;
  GdkPixbuf* pixbuf = gtk_icon_theme_load_icon(theme, icon_name, size,
                                               GtkIconLookupFlags(0), &error);
  if (!pixbuf) {
    LOG_ERROR("Failed to load icon %s: %s", icon_name,
              error ? error->message : "unknown error");
    if (error) {
      g_error_free(error);
    }
    return nullptr;
  }

  HICON icon = CreateHIconFromPixbuf(pixbuf);
  g_object_unref(pixbuf);
  return icon;
}

}  // namespace

class IptuxAppIndicatorPrivate {
 public:
  explicit IptuxAppIndicatorPrivate(IptuxAppIndicator* owner,
                                    GActionGroup* action_group)
      : owner(owner), actionGroup(action_group) {}

  ~IptuxAppIndicatorPrivate() {
    if (blinkTimerId) {
      g_source_remove(blinkTimerId);
    }
    if (iconAdded) {
      Shell_NotifyIconW(NIM_DELETE, &notifyData);
    }
    if (menu) {
      DestroyMenu(menu);
    }
    if (windowHandle) {
      DestroyWindow(windowHandle);
    }
    if (normalIcon) {
      DestroyIcon(normalIcon);
    }
    if (attentionIcon) {
      DestroyIcon(attentionIcon);
    }
    if (reverseIcon) {
      DestroyIcon(reverseIcon);
    }
    UnregisterClassW(kTrayWindowClass, GetModuleHandleW(nullptr));
  }

  IptuxAppIndicator* owner = nullptr;
  GActionGroup* actionGroup = nullptr;
  HWND windowHandle = nullptr;
  HMENU menu = nullptr;
  HICON normalIcon = nullptr;
  HICON attentionIcon = nullptr;
  HICON reverseIcon = nullptr;
  NOTIFYICONDATAW notifyData = {};
  StatusIconMode mode = STATUS_ICON_MODE_NORMAL;
  int unreadCount = 0;
  guint blinkTimerId = 0;
  bool blinkState = false;
  bool iconAdded = false;

  void ActivateAction(const char* name) {
    g_action_group_activate_action(actionGroup, name, nullptr);
  }

  void ShowMenu() {
    if (!menu || !windowHandle) {
      return;
    }
    POINT cursorPos = {};
    if (!GetCursorPos(&cursorPos)) {
      return;
    }
    SetForegroundWindow(windowHandle);
    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON,
                   cursorPos.x, cursorPos.y, 0, windowHandle, nullptr);
    PostMessageW(windowHandle, WM_NULL, 0, 0);
  }

  static LRESULT CALLBACK WndProc(HWND hwnd,
                                  UINT message,
                                  WPARAM wparam,
                                  LPARAM lparam) {
    if (message == WM_NCCREATE) {
      auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lparam);
      auto* self =
          static_cast<IptuxAppIndicatorPrivate*>(createStruct->lpCreateParams);
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
      return TRUE;
    }

    auto* self = reinterpret_cast<IptuxAppIndicatorPrivate*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (!self) {
      return DefWindowProcW(hwnd, message, wparam, lparam);
    }

    if (message == kTrayCallbackMessage) {
      const UINT event = static_cast<UINT>(lparam);
      switch (event) {
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
          self->owner->sigActivateMainWindow.emit();
          return 0;
        case WM_RBUTTONUP:
        case WM_CONTEXTMENU:
          self->ShowMenu();
          return 0;
        case WM_MOUSEWHEEL:
          self->owner->sigActivateMainWindow.emit();
          return 0;
      }
      return 0;
    }

    if (message == WM_COMMAND) {
      switch (LOWORD(wparam)) {
        case kMenuOpenId:
          self->ActivateAction("open_main_window");
          return 0;
        case kMenuPreferencesId:
          self->ActivateAction("preferences");
          return 0;
        case kMenuQuitId:
          self->ActivateAction("quit");
          return 0;
      }
    }

    return DefWindowProcW(hwnd, message, wparam, lparam);
  }
};

static bool EnsureWindow(IptuxAppIndicatorPrivate* priv) {
  if (priv->windowHandle) {
    return true;
  }

  WNDCLASSW wc = {};
  wc.lpfnWndProc = IptuxAppIndicatorPrivate::WndProc;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.lpszClassName = kTrayWindowClass;
  if (!RegisterClassW(&wc)) {
    DWORD error = GetLastError();
    if (error != ERROR_CLASS_ALREADY_EXISTS) {
      LOG_ERROR("Failed to register tray window class, error=%lu", error);
      return false;
    }
  }

  priv->windowHandle =
      CreateWindowExW(0, kTrayWindowClass, L"iptux-tray", 0, 0, 0, 0, 0,
                      HWND_MESSAGE, nullptr, GetModuleHandleW(nullptr), priv);
  if (!priv->windowHandle) {
    LOG_ERROR("Failed to create tray window, error=%lu", GetLastError());
    return false;
  }
  return true;
}

static void InitMenu(IptuxAppIndicatorPrivate* priv) {
  if (priv->menu) {
    return;
  }
  priv->menu = CreatePopupMenu();
  AppendMenuW(priv->menu, MF_STRING, kMenuOpenId,
              Utf8ToWide(_("Open Iptux")).c_str());
  AppendMenuW(priv->menu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(priv->menu, MF_STRING, kMenuPreferencesId,
              Utf8ToWide(_("Preferences")).c_str());
  AppendMenuW(priv->menu, MF_STRING, kMenuQuitId,
              Utf8ToWide(_("Quit")).c_str());
}

static HICON ResolveIconHandle(IptuxAppIndicatorPrivate* priv,
                               AppIndicatorIconState state) {
  switch (state) {
    case AppIndicatorIconState::kAttention:
      return priv->attentionIcon ? priv->attentionIcon : priv->normalIcon;
    case AppIndicatorIconState::kReverse:
      return priv->reverseIcon ? priv->reverseIcon : priv->normalIcon;
    case AppIndicatorIconState::kNormal:
      return priv->normalIcon;
  }
  return priv->normalIcon;
}

static bool AddTrayIcon(IptuxAppIndicatorPrivate* priv, HICON icon) {
  if (!EnsureWindow(priv)) {
    return false;
  }

  priv->notifyData = {};
  priv->notifyData.cbSize = sizeof(NOTIFYICONDATAW);
  priv->notifyData.hWnd = priv->windowHandle;
  priv->notifyData.uID = kTrayIconId;
  priv->notifyData.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
  priv->notifyData.uCallbackMessage = kTrayCallbackMessage;
  priv->notifyData.hIcon = icon;

  const std::wstring tip = Utf8ToWide(_("Iptux"));
  std::wcsncpy(
      priv->notifyData.szTip, tip.c_str(),
      sizeof(priv->notifyData.szTip) / sizeof(priv->notifyData.szTip[0]) - 1);
  priv->notifyData.szTip[sizeof(priv->notifyData.szTip) /
                             sizeof(priv->notifyData.szTip[0]) -
                         1] = L'\0';

  if (!Shell_NotifyIconW(NIM_ADD, &priv->notifyData)) {
    LOG_ERROR("Failed to add tray icon, error=%lu", GetLastError());
    return false;
  }

  priv->notifyData.uVersion = NOTIFYICON_VERSION_4;
  if (!Shell_NotifyIconW(NIM_SETVERSION, &priv->notifyData)) {
    LOG_ERROR("Failed to set tray icon version, error=%lu", GetLastError());
  }

  priv->iconAdded = true;
  return true;
}

static void RemoveTrayIcon(IptuxAppIndicatorPrivate* priv) {
  if (!priv->iconAdded) {
    return;
  }
  Shell_NotifyIconW(NIM_DELETE, &priv->notifyData);
  priv->iconAdded = false;
}

static void UpdateTrayIcon(IptuxAppIndicatorPrivate* priv, HICON icon) {
  if (!priv->iconAdded) {
    AddTrayIcon(priv, icon);
    return;
  }
  priv->notifyData.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
  priv->notifyData.hIcon = icon;
  if (!Shell_NotifyIconW(NIM_MODIFY, &priv->notifyData)) {
    LOG_ERROR("Failed to modify tray icon, error=%lu", GetLastError());
  }
}

static gboolean BlinkTimerCallback(gpointer data) {
  auto* priv = static_cast<IptuxAppIndicatorPrivate*>(data);
  priv->blinkState = !priv->blinkState;
  AppIndicatorIconState iconState =
      ResolveIconState(priv->mode, priv->unreadCount, priv->blinkState);
  UpdateTrayIcon(priv, ResolveIconHandle(priv, iconState));
  return G_SOURCE_CONTINUE;
}

static void StartBlinkTimer(IptuxAppIndicatorPrivate* priv) {
  if (priv->blinkTimerId) {
    return;
  }
  priv->blinkState = false;
  priv->blinkTimerId = g_timeout_add(500, BlinkTimerCallback, priv);
}

static void StopBlinkTimer(IptuxAppIndicatorPrivate* priv) {
  if (priv->blinkTimerId) {
    g_source_remove(priv->blinkTimerId);
    priv->blinkTimerId = 0;
  }
  priv->blinkState = false;
}

static void ApplyVisualState(IptuxAppIndicatorPrivate* priv) {
  if (priv->mode == STATUS_ICON_MODE_NONE) {
    StopBlinkTimer(priv);
    RemoveTrayIcon(priv);
    return;
  }

  if (ShouldBlink(priv->mode, priv->unreadCount)) {
    StartBlinkTimer(priv);
  } else {
    StopBlinkTimer(priv);
  }

  AppIndicatorIconState iconState =
      ResolveIconState(priv->mode, priv->unreadCount, priv->blinkState);
  UpdateTrayIcon(priv, ResolveIconHandle(priv, iconState));
}

IptuxAppIndicator::IptuxAppIndicator(GActionGroup* action_group) {
  priv = std::make_shared<IptuxAppIndicatorPrivate>(this, action_group);

  InitMenu(priv.get());
  priv->normalIcon = LoadIconFromTheme("iptux-icon", 32);
  priv->attentionIcon = LoadIconFromTheme("iptux-attention", 32);
  priv->reverseIcon = LoadIconFromTheme("iptux-icon-reverse", 32);

  if (!priv->normalIcon) {
    HICON fallback = LoadIcon(nullptr, IDI_APPLICATION);
    if (fallback) {
      priv->normalIcon = CopyIcon(fallback);
    }
  }

  ApplyVisualState(priv.get());
}

void IptuxAppIndicator::SetUnreadCount(int count) {
  priv->unreadCount = count;
  ApplyVisualState(priv.get());
}

void IptuxAppIndicator::SetMode(StatusIconMode mode) {
  priv->mode = mode;
  ApplyVisualState(priv.get());
}

void IptuxAppIndicator::StopBlinking() {
  StopBlinkTimer(priv.get());
  if (!priv->iconAdded || priv->mode == STATUS_ICON_MODE_NONE) {
    return;
  }
  UpdateTrayIcon(priv.get(),
                 ResolveIconHandle(priv.get(), AppIndicatorIconState::kNormal));
}

}  // namespace iptux
