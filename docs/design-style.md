# iptux Design Style Summary

## Overall Architecture — Core/GUI split

iptux is a LAN instant-messaging / file-sharing client (IP Messenger protocol, port 2425) with a deliberate layered design:

```
src/main/           → entry point, crash handler, log/i18n setup
src/api/iptux-core/ → public API headers (installed surface)
src/iptux-core/     → core implementation: protocol, networking, file transfer
src/iptux-utils/    → shared utilities (logging, string/net helpers)
src/iptux/          → GTK3 GUI layer
src/iptux-gi/       → pure C GObject layer for GIR bindings
examples/count-bot  → headless consumer of the core library (proof it's GUI-free)
```

Build order is `api → utils → core → gi → gui`, each layer only linking downward.

## Key design patterns

- **Event source / observer**: `CoreThread::signalEvent` (sigc++), carrying a typed `Event` class hierarchy (`PalEvent → NewPalOnlineEvent`, etc.). Events go down two paths: direct signal subscribers (headless bots) and a queue polled by the GUI main loop every ~100 ms — a thread-safety bridge between network threads and the UI thread.
- **Pimpl idiom**: `CoreThread::Impl` behind `std::unique_ptr`.
- **Static `create()` factories + `shared_ptr` typedefs** (`CoreThread::Ptr`, `IptuxConfig::Ptr`, fluent setters on `PalInfo`).
- **Inheritance for UI specialization**: `UiCoreThread : CoreThread` overrides pal-list hooks to mirror core state into GTK models; Chinese comments document the locking discipline ("insert: core first; delete: UI first").
- **Compile-time strategy selection**: `NotificationService` (GIO on Linux vs. TerminalNotifier on macOS), per-platform AppIndicator implementations.
- **GTK Application + GActions + .ui builder files** rather than classic MVC; custom `GtkTreeModel`s (in `UiModels.cpp`) act as the model layer.

## Codebase style

- **C++17, heavily GLib-flavored**: raw `GtkWidget*`, `g_strdup`/`g_free` idioms, C-compatible headers; `src/iptux-gi` is pure GObject C with `GError**` conventions.
- **Everything in `namespace iptux`**, with deliberate *mixed naming*: legacy `GetPal`-style methods coexist with newer lowerCamelCase API (`getName()`, `newFromString()`, `enum class`).
- **Error handling**: `iptux::Exception` with `ErrorCode` in the API layer; boolean + `getLastErr()` for lifecycle calls; `GError*` for GLib calls; log-and-continue with `LOG_*` macros for non-fatal paths; `pop_warning()` dialogs for user-facing errors.
- **Chinese-language Doxygen comments** document threading invariants and history throughout.
- **Meson + Ninja** build (`werror=true`, `cpp_std=c++17`), vendored googletest, `ldb-asynctask` subproject fallback, platform-specific source selection, glib-compile-resources for GTK assets.

## Testing

- GoogleTest, with tests **next to the source** (`*Test.cpp`) and `TestMain.cpp` per library, wired as meson `test(..., protocol: 'gtest')`.
- **GUI tests run under Xvfb** in CI; `Application` has explicit test-mode hooks (`_ForTestProcessEvents`) for deterministic main-loop driving.
- CI: ASan/UBSan, lcov/Codecov, CodeQL, multi-arch, plus GI smoke test in Python.

## i18n

Standard gettext: 16 `.po` files, `_()` macros throughout UI code, UTF-8 binding in `main/iptux.cpp`.

## In one line

A 2008-era GTK+ codebase progressively modernized into a clean C++17 core/GUI split with a typed event bus, pimpl factories, and GObject-style conventions — legacy naming and GLib idioms intentionally preserved alongside a modern, testable, headless-capable core API.
