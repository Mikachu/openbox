# ADR-004: Native Wayland Backend

## Status

**Proposed** - 2024-02-15

## Context

Wayland is becoming the default display server protocol on Linux, with major distributions (Fedora, Ubuntu, Debian) adopting it as default. Openbox is currently X11-only.

Challenges:

1. **XWayland exists**: Can run X11 apps under Wayland compositor
2. **Different paradigm**: Wayland has no "window manager" - compositor handles everything
3. **Protocol complexity**: Wayland requires implementing compositor, not just window manager

## Decision

We will **add native Wayland backend** by implementing a Wayland compositor using wlroots.

### Architecture

```
┌─────────────────────────────────────┐
│         Openbox Core                │
│  (Window management logic)          │
├─────────────────────────────────────┤
│    ┌─────────┐    ┌─────────┐       │
│    │  X11    │    │ Wayland │       │
│    │ Backend │    │Compositor│      │
│    │ (XCB)   │    │(wlroots)│       │
│    └─────────┘    └─────────┘       │
├─────────────────────────────────────┤
│         Platform Abstraction        │
└─────────────────────────────────────┘
```

### Implementation Strategy

**Phase 1**: Abstract core to be backend-agnostic
**Phase 2**: Implement wlroots-based compositor
**Phase 3**: Feature parity testing
**Phase 4**: Default enable where available

## Consequences

### Positive

- **Future-proof**: Wayland is the future of Linux graphics
- **Performance**: Direct rendering, less overhead than XWayland
- **Security**: Better isolation between applications
- **Modern features**: HDR, variable refresh rate, etc.

### Negative

- **Massive scope**: Essentially building a new window manager
- **Compatibility**: Not all apps support Wayland natively
- **NVIDIA**: Proprietary driver support still problematic
- **Maintenance**: Two backends to maintain

### Risk Mitigation

- X11 backend remains primary during transition
- Extensive testing before enabling by default
- Clear user documentation for limitations

## Alternatives Considered

### Option A: X11 Only
Continue with X11 only, rely on XWayland.

- **Rejected**: Long-term unsustainable, missing native Wayland benefits

### Option B: Labwc/Fork
Contribute to existing wlroots compositors instead.

- **Rejected**: Would lose Openbox identity and configuration ecosystem

### Option C: libei for Input
Use libei for input redirection instead of direct Wayland.

- **Under evaluation**: May be useful for testing

## Open Questions

1. How to handle Openbox-specific EWMH features on Wayland?
2. Can we maintain single configuration format for both backends?
3. What is migration path for existing users?

## Implementation

See [Wayland Backend Design](../dev-guide/wayland-backend.md) (draft).

## References

- [Wayland Book](https://wayland-book.com/)
- [wlroots](https://gitlab.freedesktop.org/wlroots/wlroots/)
- [Labwc](https://github.com/labwc/labwc) - wlroots-based Openbox-like compositor
