# Migration Guide: Openbox 3.6 to 4.0

This guide helps you migrate from Openbox 3.6 to Openbox 4.0.

## Overview

Openbox 4 is a complete rewrite with significant improvements:
- **Rust core** for memory safety and performance
- **CSS themes** replacing XML themes
- **D-Bus IPC** replacing signals
- **Extension API** for plugins
- **Python bindings** for automation

## Before You Begin

1. **Backup your configuration**:
   ```bash
   cp -r ~/.config/openbox ~/.config/openbox-3.6-backup
   ```

2. **Check compatibility**:
   ```bash
   openbox --config-test  # After installing Openbox 4
   ```

## Configuration Changes

### Theme Migration

Openbox 4 uses CSS instead of XML for themes.

**Old (3.6)**:
```xml
<theme>
  <name>Clearlooks</name>
  <titleLayout>NLIMC</titleLayout>
  <window>
    <active>
      <titlebar>
        <bg>solid</bg>
        <color>#4c4c4c</color>
      </titlebar>
    </active>
  </window>
</theme>
```

**New (4.0)**:
```css
/* theme.css */
@theme-name "Clearlooks";

window {
  --titlebar-bg: #4c4c4c;
  --titlebar-fg: #ffffff;
}

window.active {
  background: var(--titlebar-bg);
  color: var(--titlebar-fg);
}

titlebar {
  layout: "NLIMC";
}
```

**Migration tool**:
```bash
ob-theme-convert ~/.config/openbox/themes/MyTheme/themerc \
                 ~/.config/openbox/themes/MyTheme/theme.css
```

### Key Bindings

Key bindings are mostly compatible. Minor changes:

**Old**:
```xml
<keybind key="A-F4">
  <action name="Close"/>
</keybind>
```

**New** (same, but with additional features):
```xml
<keybind key="A-F4">
  <action name="Close"/>
  <action name="Execute">
    <command>notify-send "Window closed"</command>
  </action>
</keybind>
```

### Autostart

Autostart location unchanged, but new features available:

```bash
# ~/.config/openbox/autostart
# Old way (still works):
tint2 &
nm-applet &

# New: Use obctl for better integration
obctl lifecycle ready && notify-send "Openbox ready" &
```

## Control Interface Changes

### Replacing Signals with D-Bus

| Old Command | New Command |
|-------------|-------------|
| `killall -SIGUSR1 openbox` | `obctl lifecycle reconfigure` |
| `killall -SIGUSR2 openbox` | `obctl lifecycle restart` |
| `killall -SIGTERM openbox` | `obctl lifecycle exit` |
| `openbox --reconfigure` | `obctl lifecycle reconfigure` |

### New Capabilities

```bash
# List windows
obctl window list

# Focus specific window
obctl window focus 0x123456

# Switch desktop
obctl desktop switch 2

# Get/set configuration
obctl config get theme.name
obctl config set theme.name Nightfall
```

## Theme Compatibility

### Existing Themes

Openbox 4 includes a compatibility layer for 3.x themes, but recommends upgrading:

1. **Automatic conversion on first run**
2. **Manual conversion**: `ob-theme-convert`
3. **Download updated themes** from openbox.org/themes

### Custom Themes

If you created custom themes:
1. Back up your theme files
2. Run the converter
3. Test and adjust colors
4. Consider using CSS variables for easier maintenance

## Extension Compatibility

### Old Plugins

Openbox 3.x plugins (pipe menus, etc.) using the old API will not work. You need to:

1. Port to the new Extension API (C ABI)
2. Or use Python bindings for simpler scripts
3. Or use Lua scripting for configuration

### New Extensions

Openbox 4 supports:
- **C extensions** (stable ABI)
- **Python scripts** (via bindings)
- **Lua scripts** (embedded)

## Python Scripts

If you used Python with Openbox 3.x:

**Old** (using pyob):
```python
import pyob
ob = pyob.Openbox()
ob.reconfigure()
```

**New** (official bindings):
```python
import openbox
wm = openbox.WindowManager()
wm.reconfigure()
```

## Troubleshooting

### Configuration Not Loading

1. Check syntax: `openbox --config-test`
2. Check logs: `journalctl --user -u openbox`
3. Validate XML: `xmllint --noout --schema rc.xsd rc.xml`

### Themes Not Applying

1. Verify theme directory structure
2. Check CSS syntax: `obctl theme validate`
3. Try built-in theme: `obctl theme set Clearlooks`

### Performance Issues

1. Disable compositor effects if needed
2. Check extension resource usage: `obctl extension list`
3. Enable debug mode: `openbox --debug`

## Rollback Plan

If you need to return to Openbox 3.6:

1. Keep your backup: `~/.config/openbox-3.6-backup`
2. Restore: `mv ~/.config/openbox-3.6-backup ~/.config/openbox`
3. Reinstall Openbox 3.6 from distribution packages

## Getting Help

- **Documentation**: https://openbox.org/docs/4.0
- **Migration FAQ**: https://openbox.org/docs/migration-faq
- **Community**: https://discord.gg/openbox
- **Issues**: https://github.com/openbox/openbox/issues

## Timeline

- **Openbox 4.0**: Current release, migration recommended
- **Openbox 3.6**: Maintenance mode, security fixes only
- **Openbox 4.1**: Planned for Q2 2024 with additional features

## Summary

Migration is straightforward for most users:
1. Backup configuration
2. Install Openbox 4
3. Run theme converter
4. Update control scripts to use obctl
5. Enjoy the new features!

Most configurations will work with minimal changes.
