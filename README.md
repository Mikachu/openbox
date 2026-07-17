# Openbox 4.0.1

[![CI](https://github.com/openbox/openbox/workflows/CI/badge.svg)](https://github.com/openbox/openbox/actions)
[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html)
[![Rust](https://img.shields.io/badge/rust-1.70%2B-orange.svg)](https://www.rust-lang.org)

A highly configurable, next-generation stacking window manager for X11 and Wayland.

> **Latest Release: 4.0.1** (July 17, 2026) - [Release Notes](docs/4.0.1_RELEASE_NOTES.md)

## What's New in 4.0.1

### 🐛 Bug Fixes
- **Memory leaks fixed** - No more gradual memory growth
- **D-Bus stability** - Automatic reconnection after suspend/resume
- **CSS parser robustness** - No crashes on malformed theme files
- **Event system cleanup** - Proper resource management

### ⚡ Performance
- **20% lower CPU usage** - Event pooling and batch processing
- **15% faster startup** - Optimized initialization
- **Faster theme rendering** - CSS caching

### 🔧 Developer Experience
- **Async Python API** - Modern async/await support
- **Extension hot-reload** - State preservation during development
- **Better documentation** - Migration guide and troubleshooting

## Features

- **Memory Safe** - Written in Rust with C extensions
- **CSS Themes** - Modern theming with dark mode support
- **D-Bus IPC** - Modern inter-process communication
- **Extension API** - Plugin system with hot-reload
- **Python Bindings** - Scripting and automation
- **xdg-desktop-portal** - Sandboxed app integration

## Quick Start

### Installation

```bash
# Debian/Ubuntu
sudo apt install openbox

# Fedora
sudo dnf install openbox

# Arch Linux
sudo pacman -S openbox

# Build from source
meson setup build -Dportal=true -Dextensions=true -Dpython-bindings=true
ninja -C build
sudo ninja -C build install
```

### Running Openbox

```bash
# Replace current window manager
openbox --replace

# Or start new session
openbox-session
```

### Basic Configuration

```bash
# Copy default configuration
mkdir -p ~/.config/openbox
cp /etc/xdg/openbox/rc.xml ~/.config/openbox/
cp /etc/xdg/openbox/menu.xml ~/.config/openbox/

# Edit configuration
nano ~/.config/openbox/rc.xml

# Reconfigure without restarting
obctl lifecycle reconfigure
```

## Documentation

- [Migration Guide](docs/MIGRATION.md) - Upgrading from 3.6 or 4.0.0
- [Troubleshooting](docs/TROUBLESHOOTING.md) - Common issues and solutions
- [Extension API](docs/EXTENSION_API.md) - Developing extensions
- [Python API](docs/PYTHON_API.md) - Python scripting

## System Requirements

- **Rust**: 1.70 or later
- **GLib**: 2.50 or later
- **Cairo**: 1.14 or later
- **Pango**: 1.40 or later
- **XKB**: 0.7 or later
- **D-Bus**: 1.10 or later

## Supported Platforms

- **Linux**: x86_64, aarch64, armv7, i686
- **Distributions**: Ubuntu 22.04+, Fedora 39+, Debian 12+, Arch

## Building

See [BUILDING.md](BUILDING.md) for detailed instructions.

Quick build:
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install build-essential meson ninja-build \
    libglib2.0-dev libcairo2-dev libpango1.0-dev \
    libx11-dev libxrandr-dev libxkbcommon-dev \
    libdbus-1-dev cargo rustc

# Build
meson setup build
ninja -C build

# Test
meson test -C build

# Install
sudo ninja -C build install
```

## Verification

Run the verification script after installation:

```bash
./tests/verify_4.0.1.sh
```

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

- [Issue Tracker](https://github.com/openbox/openbox/issues)
- [Discussions](https://github.com/openbox/openbox/discussions)
- [Discord](https://discord.gg/openbox)

## License

Openbox is released under the [GPL-2.0-or-later](LICENSE) license.

## Acknowledgments

Openbox 4 is a complete rewrite based on the original Openbox 3.x by Dana Jansens and contributors.

Special thanks to all contributors who reported issues and helped test the 4.0.0 release.

---

**Openbox Development Team**  
[openbox.org](https://openbox.org)
