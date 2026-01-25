# GitHub Copilot Instructions for iptux

## Project Overview

iptux is a LAN communication software (IP Messenger protocol implementation) written in C++17 with a GTK+3 GUI. It enables file sharing and messaging over local area networks.

## Technology Stack

- **Language**: C++17
- **Build System**: Meson (minimum version 0.53)
- **UI Framework**: GTK+3
- **Dependencies**: GLib 2.0, jsoncpp, sigc++-2.0, ayatana-appindicator3
- **Testing Framework**: Google Test
- **Supported Platforms**: Linux (primary), macOS

## Code Style and Standards

### Code Formatting

- Use `.clang-format` configuration (based on Chromium style) for all C++ code
- Run `clang-format` on modified files before committing
- Follow EditorConfig settings:
  - 2 spaces for indentation
  - LF line endings
  - UTF-8 encoding
  - Trim trailing whitespace
  - Insert final newline

### C++ Standards

- Use C++17 features appropriately
- Enable warnings: `-Wall -Wextra -Werror`
- Follow GLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_70
- Prefer modern C++ idioms (smart pointers, RAII, etc.)

### Header Include Order

According to `.clang-format`:
1. `config.h` (if needed, priority -1)
2. Main header for source file (priority 0)
3. Other project headers (priority 1)
4. System headers ending in `.h` (priority 2)

## Building and Testing

### Build Commands

```sh
# Standard build
meson setup build
meson compile -C build

# Development build (uses resources from source directory)
meson setup -Ddev=true build
meson compile -C build

# With sanitizers
meson setup -Db_sanitize=address,undefined build
meson compile -C build
```

### Running Tests

```sh
# Run all tests
meson test -C build --verbose

# For testing on one machine with multiple instances
iptux -b 127.0.0.2 &
iptux -b 127.0.0.3 &
```

### Installation

```sh
sudo meson install -C build
```

## Project Structure

- `src/api/` - Public API headers
- `src/iptux-core/` - Core functionality (networking, protocol)
- `src/iptux/` - GTK+ UI implementation
- `src/iptux-utils/` - Utility functions
- `share/` - Application resources (icons, desktop files, AppStream metadata)
- `po/` - Internationalization/translation files
- `examples/` - Example code and scripts
- `protocol/` - Protocol documentation

## Key Architectural Concepts

- **CoreThread**: Main threading model for core functionality
- **IptuxConfig**: Configuration management
- **ProgramData**: Application state and data
- **TransFileModel**: File transfer models
- **Event**: Event handling system

## Testing Guidelines

- Use Google Test framework for unit tests
- Test files are colocated with implementation (e.g., `IptuxConfig.cpp` and `IptuxConfigTest.cpp`)
- Main test entry point: `TestMain.cpp`
- Use `TestHelper.h` for common test utilities
- Run tests with verbose output for debugging: `meson test -C build --verbose --no-stdsplit`

## Translation

- Translations managed through Weblate
- Update POT file: `meson compile update-po -C build`
- Translation files in `po/` directory

## Platform-Specific Notes

### Linux
- Primary development platform
- Supports Ubuntu 22.04, 24.04+
- Uses libayatana-appindicator3 for system tray
- Requires firewall configuration for TCP/UDP port 2425

### macOS
- Uses gtk-mac-integration for native integration
- Install dependencies via Homebrew
- Requires special loopback aliases for testing (127.0.0.2, 127.0.0.3, etc.)

## CI/CD

- GitHub Actions workflows in `.github/workflows/`
- `ci.yml`: Main CI pipeline testing on Ubuntu and macOS
- `codeql.yml`: Security scanning
- Tests run on multiple Ubuntu versions and macOS
- Both g++ and clang++ tested on Linux

## Common Development Tasks

### Adding New Features

1. Update relevant source files in `src/`
2. Add or update tests
3. Update translations if UI strings change
4. Build and test locally
5. Ensure clang-format compliance
6. Update documentation if needed

### Fixing Bugs

1. Add regression test if applicable
2. Make minimal fix in relevant source file
3. Verify tests pass
4. Check for memory leaks with sanitizers if relevant

### UI Changes

1. Update GTK+ code in `src/iptux/`
2. Test on both Linux and macOS if possible
3. Ensure accessibility features work
4. Update screenshots in documentation if needed
5. Mark strings for translation with proper i18n macros

## Important Notes

- Port 2425 (TCP/UDP) must be accessible for iptux to work
- File sending between 127.0.0.2 and 127.0.0.3 is a known limitation
- The project uses IP Messenger protocol for LAN communication
- AppStream metadata is used for software center integration
