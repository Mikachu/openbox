# Openbox 4 Documentation

Welcome to the Openbox 4 documentation! Openbox is a highly configurable, next-generation stacking window manager for X11 and Wayland.

## What is Openbox?

Openbox is a lightweight window manager that provides:

- **Speed**: Minimal resource usage, fast startup
- **Standards**: Full EWMH and ICCCM compliance
- **Customization**: Extensive configuration options
- **Modern Features**: D-Bus IPC, CSS themes, Rust core

## Quick Links

<div class=\"grid cards\" markdown>

-   :material-rocket-launch:{ .lg .middle } __Getting Started__

    ---

    Install Openbox and get your first window manager running in minutes.

    [:octicons-arrow-right-24: Installation](getting-started/installation.md)

-   :material-palette:{ .lg .middle } __Theming__

    ---

    Create beautiful window decorations with our new CSS-based theme system.

    [:octicons-arrow-right-24: Theme Guide](user-guide/theming.md)

-   :material-code-braces:{ .lg .middle } __Developer Guide__

    ---

    Contribute to Openbox, build from source, or extend with plugins.

    [:octicons-arrow-right-24: Contributing](dev-guide/contributing.md)

-   :material-api:{ .lg .middle } __API Reference__

    ---

    D-Bus API documentation for scripting and automation.

    [:octicons-arrow-right-24: D-Bus API](reference/dbus-api.md)

</div>

## What's New in Openbox 4

### 🦀 Rust Core
Complete rewrite of the window manager core in Rust for memory safety and performance.

### 🚌 D-Bus IPC
Modern D-Bus interface replaces signal-based control. Use `obctl` for command-line control.

### 🎨 CSS Themes
Theme your desktop with familiar CSS syntax, including variables and dark mode support.

### 🖥️ Wayland Support
Native Wayland backend alongside X11 support.

### ⚡ Async Event Loop
Tokio-based async architecture for better responsiveness.

## Example Configuration

```xml
<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<openbox_config xmlns=\"http://openbox.org/4.0/rc\">
  <theme>
    <name>Nightfall</name>
    <variant>dark</variant>
  </theme>
  
  <keyboard>
    <keybind key=\"A-F4\">
      <action name=\"Close\"/>
    </keybind>
    <keybind key=\"A-Tab\">
      <action name=\"NextWindow\"/>
    </keybind>
  </keyboard>
</openbox_config>
```

## Community

- 💬 [Discord](https://discord.gg/openbox)
- 🐛 [GitHub Issues](https://github.com/openbox/openbox/issues)
- 📧 [Mailing List](https://lists.openbox.org)

## License

Openbox is licensed under the GPL-2.0-or-later. See [LICENSE](https://github.com/openbox/openbox/blob/main/LICENSE) for details.
