# AGENTS.md

## Project overview

iptux is a LAN instant-messaging / file-sharing client compatible with the IP Messenger protocol (port 2425). C++17, GTK3, Meson build.

## Design & architecture

Before making significant changes, read and follow the design style summary in
[docs/design-style.md](docs/design-style.md). Key points:

- Strict core/GUI split: `src/iptux-core` must stay GUI-free; new public API headers go in `src/api/iptux-core`.
- Core communicates with the GUI through the typed `Event` hierarchy (`signalEvent` / `PopEvent` polled in the GTK main loop); do not call GTK from the core.
- Follow existing naming conventions even when mixed: lowerCamelCase for new API (`getName()`, `create()` factories), legacy PascalCase where surrounding code uses it.
- Everything lives in `namespace iptux`.
- Error handling: `iptux::Exception` with `ErrorCode` in the API layer; `GError*` for GLib calls; `LOG_*` macros for recoverable issues; `pop_warning()` dialogs for user-facing errors.

## Build & test

```sh
meson setup builddir
meson compile -C builddir
meson test -C builddir
```

- Tests use vendored GoogleTest; name test files `*Test.cpp` and place them next to the sources.
- GUI tests must not require a display (they run under Xvfb in CI).
- The build is warning-clean (`werror=true`) — fix warnings rather than suppressing them where possible.

## Running two instances locally (for testing)

To test messaging between two clients on one machine, run each instance with a
distinct bind IP and a distinct application ID (`--application-id`; without
distinct IDs, GApplication single-instance behavior makes the second GUI exit
silently):

```sh
# macOS only: the 127/8 block is not implicitly local like on Linux
sudo ifconfig lo0 alias 127.0.0.2 up

builddir/src/main/iptux -b 127.0.0.1 -a io.github.iptux_src.iptux.a -l INFO
builddir/src/main/iptux -b 127.0.0.2 -a io.github.iptux_src.iptux.b -l INFO
```

Mutual discovery and message delivery can be verified from each instance's
stdout log (`NEW_PAL_ONLINE`, received-message lines).

## Conventions

- User-visible strings must be wrapped in `_()` for gettext; the UI language is UTF-8.
- Maintain Chinese Doxygen comments where they already exist; match surrounding comment language.
- Formatting follows `.clang-format` (Chromium style, 2-space indent).
