# Openbox Extension API - Implementation Summary

## Overview

The Openbox Extension API provides a stable C ABI for developing plugins that extend Openbox functionality. This document summarizes the implementation.

## Components Delivered

### 1. xdg-desktop-portal Support

**Files:**
- `portal/openbox-portal.c` - Main portal implementation
- `portal/openbox-portal-screenshot.c` - Screenshot and color picker portal

**Implemented Portals:**
- **Settings**: Exposes theme, accent color, dark mode preference to sandboxed apps
- **Screenshot**: Full screen and window capture with permission handling
- **Background**: Controls which apps can run in background
- **Notifications**: Forwards to desktop notification daemon

### 2. systemd Integration

**Files:**
- `systemd/openbox.service` - User service unit

**Features:**
- D-Bus activation (`BusName=org.openbox.WindowManager`)
- Security hardening (sandboxing, resource limits)
- Auto-restart on failure
- Graceful shutdown via `obctl lifecycle exit`

### 3. Extension API (C ABI)

**Files:**
- `extension/openbox-extension.h` - Stable API header
- `extension/openbox-extension-loader.c` - Dynamic loader

**API Features:**
- Window management (focus, move, resize, state)
- Desktop management (switch, add, remove)
- Event subscription (window events, desktop changes)
- Configuration access
- Theme queries
- Timer system
- Cross-extension communication

**Version:** 1.0 (stable ABI)

### 4. Example Extensions

**Files:**
- `extension/examples/tiling.c` - Tiling window manager (master-stack, monocle, grid)
- `extension/examples/window-rules.c` - Automatic window rule application
- `extension/examples/compositor.c` - Basic compositor with shadows

**Build System:**
- `extension/meson.build` - Meson configuration for building extensions

### 5. Python Bindings

**Files:**
- `python/openbox/__init__.py` - Package init
- `python/openbox/core.py` - Main WindowManager class
- `python/openbox/scripting.py` - Lua/JS scripting, tiling, rules
- `python/openbox/errors.py` - Exception classes
- `python/openbox/cli.py` - Command-line interface
- `python/setup.py` - Package setup
- `python/README.md` - Documentation

**Features:**
- Full D-Bus API coverage
- Type hints and dataclasses
- Context managers
- Async support ready
- CLI tool (`ob-python`)
- Lua scripting via lupa

### 6. Display Manager Integration

**Files:**
- `display-manager/openbox.desktop` - Session desktop entry

**Supported DMs:**
- GDM (`X-GDM-SessionRegisters`)
- LightDM (`X-LightDM-Session`)
- SDDM (`X-SDDM-Session`)

## Usage Examples

### Load Extension
```bash
# Automatic loading from ~/.config/openbox/extensions/
cp tiling.so ~/.config/openbox/extensions/
# Restart Openbox or reload extensions
```

### Python Script
```python
from openbox import WindowManager

wm = WindowManager()
for w in wm.get_windows():
    print(f"{w.id}: {w.title}")
wm.switch_desktop(2)
```

### Portal Access (Sandboxed App)
```python
# Flatpak app accessing settings
from gi.repository import Gio, GLib

portal = Gio.DBusProxy.new_for_bus_sync(
    Gio.BusType.SESSION,
    Gio.DBusProxyFlags.NONE,
    None,
    'org.freedesktop.portal.Desktop',
    '/org/freedesktop/portal/desktop',
    'org.freedesktop.portal.Settings',
    None
)

# Get dark mode preference
variant = portal.call_sync('Read',
    GLib.Variant('(ss)', ('org.freedesktop.appearance', 'color-scheme')),
    Gio.DBusCallFlags.NONE, -1, None)
```

## Architecture

```
┌─────────────────────────────────────────┐
│           Applications                  │
│  (Sandboxed via Portals)                │
├─────────────────────────────────────────┤
│  Portals  │  Python  │  Extensions      │
│  Settings │  Bindings│  (C plugins)     │
│  Screenshot│         │                  │
├─────────────────────────────────────────┤
│         Openbox Core (Rust)             │
│  ┌─────────┐ ┌─────────┐ ┌──────────┐  │
│  │ Window  │ │ Desktop │ │ Extension│  │
│  │ Manager │ │ Manager │ │ Loader   │  │
│  └─────────┘ └─────────┘ └──────────┘  │
├─────────────────────────────────────────┤
│           X11 / Wayland                 │
└─────────────────────────────────────────┘
```

## Building

```bash
# Build everything
meson setup build -Dbuild-examples=true
ninja -C build

# Install Python bindings
cd python && pip install -e .

# Enable systemd service
systemctl --user enable openbox.service
```

## Future Enhancements

- More portal implementations (FileChooser, Print, etc.)
- JavaScript/QuickJS scripting backend
- Extension marketplace/distribution
- Hot-reload for extensions
- Extension configuration UI
