# Openbox Python Bindings

Python bindings for Openbox 4 window manager, providing both high-level automation APIs and scripting support.

## Installation

```bash
pip install openbox
```

Or from source:

```bash
git clone https://github.com/openbox/openbox.git
cd openbox/python
pip install -e .
```

## Quick Start

```python
import openbox

# Connect to running Openbox instance
wm = openbox.WindowManager()

# List all windows
for window in wm.get_windows():
n    print(f\"{window.id}: {window.title} on desktop {window.desktop}\")

# Focus a window
active = wm.get_active_window()
if active:
n    print(f\"Active: {active.title}\")

# Switch desktops
wm.switch_desktop(2)

# Execute a command
wm.execute(\"firefox\")
```

## Features

- **Window Management**: List, focus, move, resize, close windows
- **Desktop Management**: Switch desktops, move windows between desktops
- **Configuration**: Read/write configuration values
- **Scripting**: Lua and JavaScript support for user customization
- **Tiling**: Built-in tiling layout manager
- **Rules**: Window matching and automatic rule application

## Examples

### Window Tiling

```python
from openbox.scripting import TilingManager

tiling = TilingManager(wm)
tiling.set_layout(0, TilingManager.LAYOUT_TILED)  # Desktop 0, tiled
tiling.toggle_layout(1)  # Toggle layout on desktop 1
```

### Window Rules

```python
from openbox.scripting import WindowRules

rules = WindowRules(wm)
rules.add_rule(
n    match={'class': 'Firefox'},
n    actions={'desktop': 0, 'maximize': True}
n)
rules.add_rule(
n    match={'title': '* - Terminal'},
n    actions={'desktop': 1, 'focus': True}
n)
```

### Lua Scripting

```python
from openbox.scripting import ScriptEngine

engine = ScriptEngine(wm)
engine.run_lua(\"\"\"

-- Get all windows
local windows = openbox.get_windows()
nfor _, w in ipairs(windows) do
n    if w.class == \"Firefox\" then
n        openbox.focus_window(w.id)
n    end
nend

-- Execute command
nopenbox.execute(\"notify-send 'Hello from Lua!'\")
n\"\"\")
```

## API Reference

See [full documentation](https://openbox.org/docs/python).

## Development

```bash
# Install dev dependencies
npip install -e \".[dev]\"

# Run tests
npytest

# Type checking
nmypy openbox/

# Format code
nblack openbox/
```

## License

GPL-2.0-or-later
