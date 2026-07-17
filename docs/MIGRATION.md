# Migration Guide: Openbox 3.6 to 4.0/4.0.1

This guide helps you migrate from Openbox 3.6 to Openbox 4.0/4.0.1.

## Overview

Openbox 4 is a complete rewrite with significant improvements:
- **Rust core** for memory safety and performance
- **CSS themes** replacing XML themes
- **D-Bus IPC** replacing signals
- **Extension API** for plugins
- **Python bindings** for automation

**4.0.1 Update**: This is a stability and performance release. All users should upgrade from 4.0.0 to 4.0.1 for bug fixes.

## Before You Begin

1. **Backup your configuration**:
   ```bash
   cp -r ~/.config/openbox ~/.config/openbox-3.6-backup
   ```

2. **Check compatibility**:
   ```bash
   openbox --config-test  # After installing Openbox 4
   ```

3. **Install 4.0.1 for best experience**:
   ```bash
   # Check version
   openbox --version  # Should show 4.0.1 or higher
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

**New (4.0/4.0.1)**:
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

**4.0.1 Theme Improvements**:
- Better error messages for malformed CSS
- Automatic fallback for missing theme variants
- Improved dark mode detection

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

# New: Use obctl for better integration (4.0+)
obctl lifecycle ready && notify-send "Openbox ready" &

# 4.0.1: Better error handling
obctl lifecycle ready || notify-send "Openbox failed to start" &
```

## Control Interface Changes

### Replacing Signals with D-Bus

| Old Command | New Command |
|-------------|-------------|
| `killall -SIGUSR1 openbox` | `obctl lifecycle reconfigure` |
| `killall -SIGUSR2 openbox` | `obctl lifecycle restart` |
| `killall -SIGTERM openbox` | `obctl lifecycle exit` |
| `openbox --reconfigure` | `obctl lifecycle reconfigure` |

### New Capabilities (4.0+)

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

### Python Scripting (4.0+)

**Old** (using pyob):
```python
import pyob
ob = pyob.Openbox()
ob.reconfigure()
```

**New** (4.0/4.0.1):
```python
import openbox

# Sync API (deprecated but works)
wm = openbox.WindowManager()
wm.reconfigure()

# 4.0.1: New async API (recommended)
import asyncio
from openbox import AsyncWindowManager

async def main():
    async with AsyncWindowManager() as wm:
        windows = await wm.get_windows()
        for w in windows:
            print(f"{w.id}: {w.title}")

asyncio.run(main())
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

**4.0.1 Theme Tips**:
- Use `@media (prefers-color-scheme: dark)` for automatic dark mode
- CSS variables reduce duplication
- Test with `obctl theme validate`

## Extension Compatibility

### Old Plugins

Openbox 3.x plugins (pipe menus, etc.) using the old API will not work. You need to:

1. Port to the new Extension API (C ABI)
2. Or use Python bindings for simpler scripts
3. Or use Lua scripting for configuration

### New Extensions (4.0+)

Openbox 4 supports:
- **C extensions** (stable ABI)
- **Python scripts** (via bindings)
- **Lua scripts** (embedded)

**4.0.1 Extension Improvements**:
- Hot-reload with state preservation
- Extension-to-extension messaging
- Better error reporting with `ob_extension_get_last_error()`

## Troubleshooting

### Configuration Not Loading

1. Check syntax: `openbox --config-test`
2. Check logs: `journalctl --user -u openbox`
3. Validate XML: `xmllint --noout --schema rc.xsd rc.xml`
4. **4.0.1**: Check for migration issues in `~/.config/openbox/migration.log`

### Themes Not Applying

1. Verify theme directory structure
2. Check CSS syntax: `obctl theme validate`
3. Try built-in theme: `obctl theme set Clearlooks`
4. **4.0.1**: Check `~/.cache/openbox/theme-errors.log` for parser errors

### Performance Issues

**4.0.1 Improvements**:
- If experiencing high CPU: Update to 4.0.1 (20% CPU reduction)
- If slow startup: Check for large CSS files (>10MB)
- If memory issues: Check extension resource usage with `obctl extension list`

### D-Bus Connection Issues

**4.0.1**: Automatic reconnection added. If you still have issues:

```bash
# Check D-Bus session
echo $DBUS_SESSION_BUS_ADDRESS

# Restart D-Bus session
dbus-run-session -- openbox

# Check Openbox D-Bus service
dbus-send --session --dest=org.freedesktop.DBus \
    --type=method_call --print-reply \
    /org/freedesktop/DBus org.freedesktop.DBus.ListNames | grep openbox
```

### Python Bindings Issues

**4.0.1 Async API**:
```python
# If getting "Not connected" errors
import asyncio
from openbox import AsyncWindowManager

async def main():
    try:
        async with AsyncWindowManager() as wm:
            print(await wm.get_version())
    except openbox.ConnectionError as e:
        print(f"Connection failed: {e}")

asyncio.run(main())
```

### Extension Loading Failures

**4.0.1**:
```bash
# Check extension errors
obctl extension list --verbose

# Test extension loading
openbox --debug 2>&1 | grep -i extension

# Check dependencies
ldd ~/.config/openbox/extensions/myext.so
```

## Rollback Plan

If you need to return to Openbox 3.6:

1. Keep your backup: `~/.config/openbox-3.6-backup`
2. Restore: `mv ~/.config/openbox-3.6-backup ~/.config/openbox`
3. Reinstall Openbox 3.6 from distribution packages

**4.0.1**: Can coexist with 3.6 - use `openbox3` vs `openbox` if both installed.

## Getting Help

- **Documentation**: https://openbox.org/docs/4.0
- **Migration FAQ**: https://openbox.org/docs/migration-faq
- **Community**: https://discord.gg/openbox
- **Issues**: https://github.com/openbox/openbox/issues

**4.0.1 Specific**:
- Check `CHANGELOG.md` for all bug fixes
- Review updated man pages: `man openbox`, `man obctl`
- See `docs/EXTENSION_API.md` for extension development

## Timeline

- **Openbox 4.0.1**: Current stable release (January 2024)
- **Openbox 4.0**: Initial release (January 2024) - update recommended
- **Openbox 3.6**: Maintenance mode, security fixes only

## Summary

Migration is straightforward for most users:
1. Backup configuration
2. Install Openbox 4.0.1 (recommended over 4.0.0)
3. Run theme converter
4. Update control scripts to use obctl
5. Enjoy the new features!

Most configurations work with minimal changes. The 4.0.1 release fixes all known migration issues from 4.0.0.
