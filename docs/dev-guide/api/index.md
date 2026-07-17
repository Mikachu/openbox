# Openbox API Documentation

Complete API reference for Openbox 4.

## API Overview

Openbox 4 provides multiple APIs for different use cases:

| API | Purpose | Language |
|-----|---------|----------|
| **D-Bus** | Runtime control and monitoring | Any |
| **C FFI** | Plugin development | C/C++ |
| **Rust** | Core development | Rust |
| **Configuration** | Static configuration | XML/CSS |

## D-Bus API

The primary interface for controlling Openbox at runtime.

### Service

```
norg.openbox.WindowManager
n  /org/openbox/WindowManager
```

### Quick Example

```python
nimport dbus

nbus = dbus.SessionBus()
nwm = bus.get_object('org.openbox.WindowManager',
n                    '/org/openbox/WindowManager')

n# Reconfigure
nwm.Reconfigure()

n# List windows
nwindows = wm.GetWindows(dbus_interface='org.openbox.WindowManager')
n```

### Documentation

- [D-Bus API Reference](../reference/dbus-api.md)
- [obctl(1)](../man/obctl.1.md) - Command-line tool

## C FFI

For writing plugins and extensions.

### Header

```c
n#include <openbox/plugin.h>
n```

### Example Plugin

```c
n#include <openbox/plugin.h>

nOB_PLUGIN_EXPORT int ob_plugin_init(void) {
n    ob_log(OB_LOG_INFO, \"My plugin loaded\");
n    return 0;
n}

nOB_PLUGIN_EXPORT void ob_plugin_fini(void) {
n    ob_log(OB_LOG_INFO, \"My plugin unloaded\");
n}
n```

### Documentation

- [Plugin API Reference](plugin-api.md)
- [Writing Plugins](../tutorials/writing-plugins.md)

## Rust API

Internal API for core development.

### Modules

| Module | Purpose |
|--------|---------|
| `ob_core::client` | Window client management |
| `ob_core::focus` | Focus tracking |
| `ob_core::stack` | Window stacking |
| `ob_core::config` | Configuration |
| `ob_core::event` | Event handling |

### Example

```rust
nuse ob_core::client::ClientManager;

nasync fn setup_wm() -> Result<(), Box<dyn Error>> {
n    let clients = ClientManager::new().await?;
n    // ...
n    Ok(())
n}
n```

### Documentation

- [Rust API Docs](rust-api/index.html) (cargo doc)
- [Architecture Guide](../architecture.md)

## Configuration API

Programmatic configuration access.

### Reading Configuration

```python
n# Using D-Bus
nimport dbus

nbus = dbus.SessionBus()
nwm = bus.get_object('org.openbox.WindowManager',
n                    '/org/openbox/WindowManager')
nconfig = dbus.Interface(wm, 'org.openbox.Config')

ntheme = config.Get('theme.name')
n```

### Writing Configuration

Direct file modification with validation:

```bash
n# Edit config
n$EDITOR ~/.config/openbox/rc.xml

n# Validate
nobctl config validate
n```

## API Stability

| API | Stability | Notes |
|-----|-----------|-------|
| D-Bus | Stable | Semantic versioning |
| C FFI | Stable | Backward compatible |
| Rust | Unstable | May change between minors |
| Config | Stable | XML stable, CSS evolving |

## Generating Documentation

### Doxygen (C/C++)

```bash
ndoxygen Doxyfile
n```

### Rust

```bash
ncargo doc --workspace --no-deps --open
n```

### Python

```bash
npdoc3 --html openbox_dbus/
n```

## See Also

- [Contributing Guide](../contributing.md)
- [Architecture Overview](../architecture.md)
- [D-Bus Specification](https://dbus.freedesktop.org/doc/dbus-specification.html)
