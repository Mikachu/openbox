# Openbox 4

[![CI](https://github.com/openbox/openbox/workflows/CI/badge.svg)](https://github.com/openbox/openbox/actions)
[![License](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](LICENSE)

A highly configurable, next-generation stacking window manager for X11 and Wayland.

## Features

- **Memory Safe**: Core rewritten in Rust, eliminating entire classes of bugs
- **Modern Theming**: CSS-based themes with variables and dark mode support
- **Rich IPC**: D-Bus interface for control and monitoring
- **Extensible**: Stable C ABI for plugins, Python bindings for automation
- **Desktop Integration**: xdg-desktop-portal support, systemd integration
- **Cross-Platform**: Native X11 and Wayland backends

## Quick Start

### Installation

```bash
# Debian/Ubuntu
sudo apt install openbox

# Fedora
sudo dnf install openbox

# Arch Linux
sudo pacman -S openbox

# Flatpak
flatpak install flathub org.openbox.Openbox

# AppImage (universal)
wget https://openbox.org/releases/Openbox-4.0.0-x86_64.AppImage
chmod +x Openbox-4.0.0-x86_64.AppImage
./Openbox-4.0.0-x86_64.AppImage
```

### Building from Source

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install build-essential meson ninja-build rustc cargo \
    libxml2-dev libglib2.0-dev libcairo2-dev libpango1.0-dev \
    libx11-dev libxinerama-dev libxrandr-dev

# Build
meson setup build --buildtype=release
ninja -C build

# Install
sudo ninja -C build install
```

### Configuration

```bash
# Copy default config
mkdir -p ~/.config/openbox
cp /etc/xdg/openbox/rc.xml ~/.config/openbox/

# Edit configuration
nano ~/.config/openbox/rc.xml

# Reconfigure
obctl lifecycle reconfigure
```

## Documentation

- [Installation Guide](docs/getting-started/installation.md)
- [Configuration Guide](docs/user-guide/configuration.md)
- [Theming Guide](docs/user-guide/theming.md)
- [Extension API](extension/README.md)
- [Python Bindings](python/README.md)
- [Migration from 3.x](docs/MIGRATION.md)

## Usage

```bash
# Start Openbox
openbox --replace

# Control via obctl
obctl window list                    # List windows
obctl window focus 0x123456         # Focus window
obctl desktop switch 2              # Switch desktop
obctl lifecycle reconfigure         # Reload config

# Python scripting
python3 -c "
import openbox
wm = openbox.WindowManager()
for w in wm.get_windows():
    print(f'{w.id}: {w.title}')
"
```

## Contributing

We welcome contributions! See [Contributing Guide](docs/dev-guide/contributing.md) for details.

```bash
# Clone and build
git clone https://github.com/openbox/openbox.git
cd openbox
meson setup build
ninja -C build

# Run tests
./scripts/run-tests.sh
```

## License

GPL-2.0-or-later. See [LICENSE](LICENSE) for details.

## Links

- Website: https://openbox.org
- Documentation: https://openbox.org/docs
- Issues: https://github.com/openbox/openbox/issues
- Discussions: https://github.com/openbox/openbox/discussions
- Discord: https://discord.gg/openbox

---

**Note**: Openbox 4 is a complete rewrite. If upgrading from 3.x, see the [Migration Guide](docs/MIGRATION.md).
