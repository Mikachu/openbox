# Changelog

All notable changes to Openbox will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [4.0.0] - 2024-01-15

### Added

#### Core
- Complete rewrite of window manager core in Rust
- Memory safety guarantees via Rust's ownership system
- Async event loop using Tokio for better responsiveness
- Zero-cost abstractions maintaining lightweight philosophy

#### Theming
- CSS-based theme system replacing XML
- CSS variables for theme-wide values
- Media queries for dark mode support (`prefers-color-scheme`)
- Nested selectors for cleaner theme files
- Built-in theme converter from 3.x format

#### IPC and Control
- D-Bus service (`org.openbox.WindowManager`) replacing signals
- `obctl` command-line utility for control
- Rich API for querying and controlling all aspects
- Python bindings with full API coverage
- Event signals for monitoring

#### Extensions
- Stable C ABI for plugin development
- Extension loader with hot-reload support
- Event subscription system
- Example extensions: tiling, window rules, compositor

#### Desktop Integration
- xdg-desktop-portal support for sandboxed apps
  - Settings portal (theme, dark mode)
  - Screenshot portal
  - Background portal
  - Notification portal
- systemd user service integration
- Display manager integration (GDM, LightDM, SDDM)

#### Scripting
- Lua scripting support via lupa
- JavaScript support (planned)
- Window rules engine
- Tiling layout manager

#### Packaging
- Flatpak manifest for sandboxed distribution
- AppImage build script for universal binary
- Modern Debian/Ubuntu packaging
- RPM spec for Fedora/openSUSE
- Homebrew formula for macOS

### Changed

- Build system: Autotools → Meson
- Theme format: XML → CSS
- IPC: Signals → D-Bus
- Minimum Rust version: 1.70
- Configuration: Mostly compatible with 3.x

### Deprecated

- Unix signal interface (SIGUSR1, SIGUSR2) - use D-Bus instead
- XML theme format - use CSS instead
- Old Python bindings - use new official bindings

### Removed

- Direct Xlib dependencies in core (now via XCB)
- Deprecated configuration options

### Fixed

- Memory leaks in long-running sessions
- Race conditions in event handling
- Various crash bugs from memory safety issues

### Security

- Eliminated entire classes of memory safety vulnerabilities
- Sandboxed app support via portals
- Reduced attack surface in core

## [3.6.1] - 2023-06-15

### Fixed
- Crash on certain multi-monitor configurations
- Theme parsing edge cases

## [3.6.0] - 2022-01-01

Last release of Openbox 3.x series.

[4.0.0]: https://github.com/openbox/openbox/compare/v3.6.1...v4.0.0
