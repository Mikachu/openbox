# Openbox Extension System

The Openbox Extension API provides a stable C ABI for developing plugins that extend Openbox functionality.

## Quick Start

### Creating an Extension

1. Include the extension header:
```c
#include <openbox/openbox-extension.h>
```

2. Implement required entry points:
```c
OB_EXTENSION_EXPORT const ObExtensionInfo* ob_extension_get_info(void);
nOB_EXTENSION_EXPORT bool ob_extension_init(ObExtension *ext, const ObApi *api);
nOB_EXTENSION_EXPORT void ob_extension_fini(ObExtension *ext);
```

3. Build as shared library:
```bash
gcc -shared -fPIC -o myext.so myext.c
```

4. Install to extensions directory:
```bash
cp myext.so ~/.config/openbox/extensions/
```

## API Overview

### Window Management
- `window_focus()`, `window_raise()`, `window_lower()`
- `window_move()`, `window_resize()`, `window_set_geometry()`
- `window_maximize()`, `window_minimize()`, `window_close()`

### Desktop Management
- `desktop_get_count()`, `desktop_get_current()`
- `desktop_set_current()`, `desktop_add()`, `desktop_remove()`

### Events
Subscribe to window events:
```c
ob_api->event_subscribe(ext, OB_EVENT_WINDOW_CREATE, callback, user_data);
```

### Configuration
- `config_get_string()`, `config_get_int()`, `config_get_bool()`
- `config_set()`, `config_reload()`

## Examples

See `examples/` directory:
- `tiling.c` - Tiling window manager
- `window-rules.c` - Automatic window rules
- `compositor.c` - Basic compositor effects

## Documentation

- [API Reference](https://openbox.org/docs/extension-api)
- [Tutorial: Writing Extensions](https://openbox.org/docs/tutorials/writing-extensions)
