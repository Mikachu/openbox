# ADR-002: Replace Signals with D-Bus IPC

## Status

**Accepted** - 2024-01-20

## Context

Openbox 3.x uses Unix signals for IPC:
- `SIGUSR1` for reconfigure
- `SIGUSR2` for restart  
- `SIGTERM` for exit

This approach has limitations:

1. **No feedback**: Signals are fire-and-forget
2. **No data exchange**: Can't query state or send complex commands
3. **Race conditions**: Signal handling is asynchronous and tricky
4. **Limited extensibility**: Can't add new functionality without new signals

Modern desktop environments expect rich IPC via D-Bus.

## Decision

We will **implement a D-Bus service** (`org.openbox.WindowManager`) that provides:

- **Methods**: Reconfigure, Restart, Exit, GetWindows, FocusWindow, etc.
- **Signals**: WindowCreated, WindowFocused, DesktopChanged, etc.
- **Properties**: Version, Theme, CurrentDesktop

Legacy signal interface will be **deprecated but maintained** for backward compatibility.

### Interface Design

```xml
<interface name="org.openbox.WindowManager">
  <method name="Reconfigure"/>
  <method name="GetWindows">
    <arg direction="out" type="a(ua{sv})"/>
  </method>
  <signal name="WindowCreated">
    <arg type="u" name="window_id"/>
  </signal>
</interface>
```

## Consequences

### Positive

- **Rich API**: Query and control all aspects of window manager
- **Type safety**: D-Bus provides type checking
- **Introspection**: Self-documenting interface
- **Tooling**: Works with `dbus-send`, `gdbus`, Python, etc.
- **Signals**: Events for scripts and monitoring tools

### Negative

- **Dependency**: Requires D-Bus session bus
- **Complexity**: More complex than signals
- **Overhead**: Slight runtime overhead (acceptable)

### Migration

| Legacy | New |
|--------|-----|
| `killall -SIGUSR1 openbox` | `obctl lifecycle reconfigure` |
| `killall -SIGUSR2 openbox` | `obctl lifecycle restart` |
| `killall -SIGTERM openbox` | `obctl lifecycle exit` |
| N/A | `obctl window list` |
| N/A | `obctl window focus 12345` |

## Alternatives Considered

### Option A: Keep Signals
Add more signals for new functionality.

- **Rejected**: Doesn't solve fundamental limitations

### Option B: Custom Socket Protocol
Unix domain socket with custom protocol.

- **Rejected**: Reinventing D-Bus, no standard tooling

### Option C: HTTP/REST
HTTP API for control.

- **Rejected**: Overkill, security concerns, no desktop integration

## Implementation

See [D-Bus API Documentation](../reference/dbus-api.md) and [obctl Manual](../man/obctl.1.md).

## References

- [D-Bus Specification](https://dbus.freedesktop.org/doc/dbus-specification.html)
- [freedesktop.org Window Manager Specification](https://specifications.freedesktop.org/wm-spec/)
