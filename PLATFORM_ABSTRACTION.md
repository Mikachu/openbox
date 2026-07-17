# Openbox Platform Abstraction Layer (PAL)

## Overview

The Platform Abstraction Layer (PAL) provides a unified interface for supporting multiple display backends (X11 and Wayland) in Openbox. This enables Openbox to run natively on both traditional X11 systems and modern Wayland compositors.

## Architecture

```
openbox/platform/
├── ob_platform.h          # Core platform interface
├── ob_platform.c          # Platform dispatcher
├── meson.build            # Build configuration
├── x11/                   # X11 backend
│   ├── x11_backend.h
│   ├── x11_display.c
│   ├── x11_events.c
│   └── x11_window.c
└── wayland/               # Wayland backend
    ├── wayland_backend.h
    ├── wayland_display.c
    ├── wayland_window.c
    └── (wayland_events.c - TODO)
```

## Key Components

### 1. Platform Types (`ObPlatformType`)
- `OB_PLATFORM_X11` - X11 backend
- `OB_PLATFORM_WAYLAND` - Wayland backend
- `OB_PLATFORM_NONE` - Auto-detect

### 2. Core Abstractions

#### ObPlatformDisplay
Represents a connection to the display server:
- **X11**: `Display*` from Xlib
- **Wayland**: `wl_display*` from wayland-client

#### ObPlatformWindow
Represents a window/surface:
- **X11**: `Window` ID + `XWindowAttributes`
- **Wayland**: `wl_surface*` + `xdg_surface*` + `xdg_toplevel*`

#### ObPlatformSurface
Represents decoration frames:
- **X11**: Subwindows for titlebar, buttons, etc.
- **Wayland**: `wl_subsurface*` for frame elements

#### ObPlatformSeat
Represents input devices:
- **X11**: Single seat with pointer/keyboard
- **Wayland**: Multiple seats with hotplug support

#### ObPlatformMonitor
Represents display outputs:
- **X11**: Xinerama/XRandR outputs
- **Wayland**: `wl_output` objects

### 3. Event System

Platform-agnostic event types:
```c
typedef enum {
    OB_EVENT_EXPOSE,
    OB_EVENT_CONFIGURE,
    OB_EVENT_MAP,
    OB_EVENT_UNMAP,
    OB_EVENT_BUTTON_PRESS,
    OB_EVENT_BUTTON_RELEASE,
    OB_EVENT_MOTION,
    OB_EVENT_KEY_PRESS,
    OB_EVENT_KEY_RELEASE,
    OB_EVENT_OUTPUT_CHANGE,
    // ... etc
} ObEventType;
```

Events are translated from native format to `ObPlatformEvent` structure.

## Backend Selection

### Auto-Detection (Default)
```c
ob_platform_select_backend(OB_PLATFORM_NONE);
// Tries Wayland first, falls back to X11
```

### Explicit Selection
```c
ob_platform_select_backend(OB_PLATFORM_WAYLAND);
// or
ob_platform_select_backend(OB_PLATFORM_X11);
```

### Environment Variables
- `DISPLAY` - Triggers X11 backend
- `WAYLAND_DISPLAY` - Triggers Wayland backend

## API Usage

### Initialization
```c
ob_platform_init();
ob_platform_select_backend(OB_PLATFORM_NONE);
ob_platform_display_open(NULL);
```

### Creating Windows
```c
ObWindowAttributes attrs = {
    .x = 100, .y = 100,
    .width = 800, .height = 600,
    .title = "My Window",
    .type = OB_WINDOW_TYPE_NORMAL
};

ObPlatformWindow *window;
OB_PLATFORM_CALL(window_create, display, NULL, &attrs, &window);
```

### Event Handling
```c
ObPlatformEvent event;
while (OB_PLATFORM_CALL(event_poll, display, &event)) {
    switch (event.type) {
        case OB_EVENT_BUTTON_PRESS:
            handle_button_press(&event.data.button);
            break;
        case OB_EVENT_CONFIGURE:
            handle_resize(&event.data.configure);
            break;
        // ... etc
    }
    OB_PLATFORM_CALL(event_free, &event);
}
```

## X11 Backend Details

### Extensions Supported
- XKB (keyboard)
- X Shape (non-rectangular windows)
- Xinerama (multi-monitor)
- XRandR (hotplug, resolution changes)
- X Sync (synchronization)

### Features
- Full ICCCM/EWMH compliance
- Window manager hints
- Startup notification
- Session management

## Wayland Backend Details

### Protocols Used
- `wl_compositor` - Surface creation
- `xdg_wm_base` - Window management
- `zxdg_decoration_manager_v1` - Server-side decorations
- `wl_output` - Monitor information
- `wl_seat` - Input devices

### Design Considerations

1. **Positioning**: Wayland clients cannot set their own position. The compositor decides window placement.

2. **Sizing**: Windows receive configure events from the compositor and must respect the suggested size.

3. **Decorations**: Server-side decorations are preferred (via xdg-decoration). Client-side decorations can be implemented as subsurfaces.

4. **Focus**: Explicit focus requests may not be honored. The compositor controls focus.

5. **Global coordinates**: Wayland uses surface-local coordinates. Global coordinates are not available.

## Migration Strategy

### Phase 1: Abstraction Layer (Current)
- Create PAL alongside existing code
- X11 backend uses existing code paths
- Wayland backend is stubbed/functional

### Phase 2: Refactoring
- Move X11-specific code from `openbox/*.c` to `openbox/platform/x11/`
- Update callers to use PAL API
- Maintain backward compatibility

### Phase 3: Full Wayland Support
- Complete Wayland protocol implementations
- Testing on major compositors (Sway, Weston, etc.)
- Documentation and packaging

## Build Configuration

### Meson Options
```bash
meson setup builddir \
  -Dx11=enabled \
  -Dwayland=enabled
```

### Dependencies

**X11 Backend:**
- libx11
- libxext (for extensions)
- libxinerama (optional)
- libxrandr (optional)
- libxkbcommon-x11 (optional)

**Wayland Backend:**
- wayland-client
- wayland-cursor
- wayland-protocols (build-time)
- libxkbcommon

## Testing

### X11 Testing
```bash
# Traditional X11 session
./openbox --replace

# XWayland (nested)
GDK_BACKEND=x11 ./openbox
```

### Wayland Testing
```bash
# On a Wayland compositor
./openbox

# Explicit backend
./openbox --backend=wayland
```

## Known Limitations

### Wayland Limitations
1. No global coordinates (affects placement algorithms)
2. No window raising/lowering (compositor controlled)
3. No traditional desktop concept (workspaces are compositor-specific)
4. Limited window manager control (by design)

### Workarounds
- Use layer-shell protocol for panels/docks
- Implement as a compositor plugin for full control
- Use xdg-portals for specific features

## Future Enhancements

1. **Vulkan rendering** - Replace XRender with Vulkan
2. **DMA-BUF** - Zero-copy buffer sharing
3. **Presentation feedback** - Better frame timing
4. **Color management** - HDR support
5. **Fractional scaling** - Better HiDPI support

## References

- [Wayland Book](https://wayland-book.com/)
- [XDG Shell Protocol](https://gitlab.freedesktop.org/wayland/wayland-protocols/-/blob/main/stable/xdg-shell/xdg-shell.xml)
- [ICCCM](https://tronche.com/gui/x/icccm/)
- [EWMH](https://specifications.freedesktop.org/wm-spec/wm-spec-latest.html)
