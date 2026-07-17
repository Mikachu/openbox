# Openbox D-Bus API Documentation

## Overview

Openbox 4 provides a comprehensive D-Bus API for controlling the window manager, replacing the legacy signal-based interface (`openbox --reconfigure`, etc.) with a modern, introspectable, and extensible IPC mechanism.

## Bus Information

- **Bus Name**: `org.openbox.WindowManager`
- **Object Path**: `/org/openbox/WindowManager`
- **Interface**: `org.openbox.WindowManager`

## Quick Start

### Using obctl (Command Line)

```bash
# Get status
obctl status

# List windows
obctl list

# Focus a window
obctl window focus 12345

# Switch desktop
obctl desktop switch 2

# Reconfigure
obctl lifecycle reconfigure
```

### Using Python

```python
from openbox_dbus import OpenboxClient

with OpenboxClient() as client:
    # List windows
    windows = client.get_windows()
    
    # Focus window
    client.focus_window(12345)
    
    # Switch desktop
    client.set_current_desktop(2)
```

### Using D-Bus directly

```bash
# Get windows
dbus-send --session --dest=org.openbox.WindowManager \
    --type=method_call --print-reply \
    /org/openbox/WindowManager \
    org.openbox.WindowManager.GetWindows

# Reconfigure
dbus-send --session --dest=org.openbox.WindowManager \
    --type=method_call \
    /org/openbox/WindowManager \
    org.openbox.WindowManager.Reconfigure
```

## Methods

### Lifecycle Control

#### Reconfigure()
Reload configuration files without restarting.

**Returns**: nothing

#### Restart(save_session: b)
Restart the window manager.

**Parameters**:
- `save_session` (bool): Whether to save session state

**Returns**: nothing

#### Exit(save_session: b)
Exit the window manager.

**Parameters**:
- `save_session` (bool): Whether to save session state

**Returns**: nothing

### Window Management

#### GetWindows() → a(ua{sv})
Get list of all managed windows.

**Returns**: Array of tuples:
- Window ID (uint32)
- Properties dictionary (string → variant)

**Properties include**:
- `title` (string): Window title
- `class` (string): Window class
- `desktop` (int32): Desktop number
- `states` (array of strings): Window states
- `geometry` (array of 4 ints): x, y, width, height
- `mapped` (bool): Whether window is mapped
- `managed` (bool): Whether window is managed

#### GetWindow(window_id: u) → a{sv}
Get detailed properties of a specific window.

**Parameters**:
- `window_id` (uint32): Window ID

**Returns**: Properties dictionary

#### FocusWindow(window_id: u, raise: b)
Set focus to a window.

**Parameters**:
- `window_id` (uint32): Window to focus
- `raise` (bool): Whether to raise the window

#### MoveWindow(window_id: u, x: i, y: i)
Move a window.

**Parameters**:
- `window_id` (uint32): Window to move
- `x` (int32): New X coordinate
- `y` (int32): New Y coordinate

#### ResizeWindow(window_id: u, width: u, height: u)
Resize a window.

**Parameters**:
- `window_id` (uint32): Window to resize
- `width` (uint32): New width
- `height` (uint32): New height

#### CloseWindow(window_id: u)
Close a window.

**Parameters**:
- `window_id` (uint32): Window to close

#### IconifyWindow(window_id: u)
Minimize/iconify a window.

**Parameters**:
- `window_id` (uint32): Window to iconify

#### MaximizeWindow(window_id: u, full: b)
Maximize a window.

**Parameters**:
- `window_id` (uint32): Window to maximize
- `full` (bool): Whether to maximize in both directions

#### UnmaximizeWindow(window_id: u)
Restore a maximized window.

**Parameters**:
- `window_id` (uint32): Window to unmaximize

#### ShadeWindow(window_id: u)
Shade (roll up) a window.

**Parameters**:
- `window_id` (uint32): Window to shade

#### UnshadeWindow(window_id: u)
Unshade a window.

**Parameters**:
- `window_id` (uint32): Window to unshade

#### SetWindowDesktop(window_id: u, desktop: i, follow: b)
Move window to a different desktop.

**Parameters**:
- `window_id` (uint32): Window to move
- `desktop` (int32): Target desktop number
- `follow` (bool): Whether to switch to that desktop

#### SetWindowLayer(window_id: u, layer: s)
Set window stacking layer.

**Parameters**:
- `window_id` (uint32): Window to modify
- `layer` (string): Layer name ("normal", "above", "below")

### Desktop Management

#### GetDesktops() → a(is)
Get list of desktops.

**Returns**: Array of tuples:
- Desktop number (int32)
- Desktop name (string)

#### GetCurrentDesktop() → i
Get current desktop number.

**Returns**: Desktop number (int32)

#### SetCurrentDesktop(desktop: i)
Switch to a desktop.

**Parameters**:
- `desktop` (int32): Desktop number

#### AddDesktop(name: s) → i
Add a new desktop.

**Parameters**:
- `name` (string): Desktop name

**Returns**: New desktop number

#### RemoveDesktop(desktop: i)
Remove a desktop.

**Parameters**:
- `desktop` (int32): Desktop number to remove

#### RenameDesktop(desktop: i, name: s)
Rename a desktop.

**Parameters**:
- `desktop` (int32): Desktop number
- `name` (string): New name

### Configuration

#### GetConfig(key: s) → v
Get configuration value.

**Parameters**:
- `key` (string): Configuration key

**Returns**: Configuration value (variant)

#### SetConfig(key: s, value: v, persistent: b)
Set configuration value.

**Parameters**:
- `key` (string): Configuration key
- `value` (variant): New value
- `persistent` (bool): Whether to save to config file

### Theme

#### GetTheme() → s
Get current theme name.

**Returns**: Theme name (string)

#### SetTheme(theme_name: s)
Set theme.

**Parameters**:
- `theme_name` (string): Theme name

### Actions

#### ShowMenu(menu_name: s, x: i, y: i)
Show a menu.

**Parameters**:
- `menu_name` (string): Menu identifier
- `x` (int32): X coordinate
- `y` (int32): Y coordinate

#### Execute(command: s, args: as)
Execute a command.

**Parameters**:
- `command` (string): Command to execute
- `args` (array of strings): Command arguments

## Signals

### WindowCreated(window_id: u, properties: a{sv})
Emitted when a new window is created.

### WindowDestroyed(window_id: u)
Emitted when a window is destroyed.

### WindowFocused(window_id: u, previous_window_id: u)
Emitted when focus changes.

### WindowPropertyChanged(window_id: u, property_name: s, new_value: v)
Emitted when a window property changes.

### DesktopChanged(new_desktop: i, old_desktop: i)
Emitted when the active desktop changes.

### ConfigChanged(key: s, new_value: v)
Emitted when configuration changes.

### ThemeChanged(theme_name: s)
Emitted when the theme changes.

## Properties

### Version → s (read-only)
Openbox version string.

### ApiVersion → u (read-only)
D-Bus API version number (currently 1).

## Legacy Compatibility

The legacy signal interface is still supported but deprecated:

| Legacy | D-Bus Equivalent |
|--------|-----------------|
| `openbox --reconfigure` | `obctl lifecycle reconfigure` |
| `openbox --restart` | `obctl lifecycle restart` |
| `openbox --exit` | `obctl lifecycle exit` |
| `openbox --debug` | `obctl config set debug true` |

## Introspection

Get full interface definition:

```bash
dbus-send --session --dest=org.openbox.WindowManager \
    --type=method_call --print-reply \
    /org/openbox/WindowManager \
    org.freedesktop.DBus.Introspectable.Introspect
```

Or use `busctl`:

```bash
busctl --user introspect org.openbox.WindowManager \
    /org/openbox/WindowManager
```

## Error Handling

Errors are returned as D-Bus errors:

- `org.freedesktop.DBus.Error.InvalidArgs`: Invalid arguments
- `org.freedesktop.DBus.Error.Failed`: Operation failed
- `org.freedesktop.DBus.Error.NotSupported`: Feature not supported

## Security

The D-Bus service uses standard D-Bus policy files. By default:
- Any user can query window information
- Only session owner can modify windows
- Lifecycle control may require additional permissions

Policy file location: `/usr/share/dbus-1/system.d/org.openbox.WindowManager.conf`

## Python API Reference

See `tools/obctl-py/openbox_dbus.py` for full Python API.

### Example: Event Monitoring

```python
from openbox_dbus import OpenboxClient, OpenboxEventListener

class MyListener(OpenboxEventListener):
    def _on_window_created(self, window_id, properties):
        print(f\"Window created: {properties.get('title')}\")
    
    def _on_window_focused(self, window_id, previous_id):
        print(f\"Focus changed to: {window_id}\")

listener = MyListener()
listener.start()  # Blocks and processes events
```

### Example: Window Tiling Script

```python
from openbox_dbus import OpenboxClient

def tile_windows():
    with OpenboxClient() as client:
        windows = client.get_windows()
        screen_width = 1920
        screen_height = 1080
        
        for i, win in enumerate(windows):
            x = (i % 2) * (screen_width // 2)
            y = (i // 2) * (screen_height // 2)
            client.move_window(win['id'], x, y)
            client.resize_window(win['id'], 
                               screen_width // 2, 
                               screen_height // 2)

tile_windows()
```

## See Also

- [obctl Manual](OBDCTL.md)
- [Python Bindings](PYTHON_API.md)
- [D-Bus Specification](https://dbus.freedesktop.org/doc/dbus-specification.html)
