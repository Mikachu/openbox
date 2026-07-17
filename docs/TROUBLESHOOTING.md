# Openbox 4.0.1 Troubleshooting Guide

Common issues and solutions for Openbox 4.0.1.

## Quick Diagnostics

```bash
# Check Openbox version
openbox --version

# Test configuration
openbox --config-test

# Check D-Bus connection
obctl lifecycle status

# View logs
journalctl --user -u openbox -f
```

## Common Issues

### 1. Openbox Won't Start

**Symptoms**: Black screen, returns to login manager, or freezes.

**Solutions**:

```bash
# Check for D-Bus session
if [ -z "$DBUS_SESSION_BUS_ADDRESS" ]; then
    eval $(dbus-launch --sh-syntax)
fi

# Test with debug output
openbox --debug --replace 2>&1 | tee /tmp/openbox-debug.log

# Check for missing dependencies
ldd $(which openbox) | grep "not found"

# Try minimal config
XDG_CONFIG_HOME=/tmp/openbox-test openbox --replace
```

### 2. Themes Not Loading

**Symptoms**: Windows have no decorations, or default ugly theme.

**Solutions**:

```bash
# Validate theme CSS
obctl theme validate

# Check theme directory
ls -la ~/.config/openbox/themes/

# Test with built-in theme
obctl theme set Clearlooks

# Check for CSS errors (4.0.1)
cat ~/.cache/openbox/theme-errors.log

# Fix permissions
chmod -R u+rw ~/.config/openbox/themes/
```

### 3. Key Bindings Not Working

**Symptoms**: Keyboard shortcuts have no effect.

**Solutions**:

```bash
# Test configuration
openbox --config-test

# Check for syntax errors
xmllint --noout ~/.config/openbox/rc.xml

# Reconfigure
obctl lifecycle reconfigure

# Check for conflicting shortcuts
obctl keybindings list  # If available
```

### 4. Python Bindings Not Working

**Symptoms**: Python scripts fail with connection errors.

**Solutions**:

```bash
# Check Python version (3.8+ required)
python3 --version

# Check if Openbox is running
pgrep -x openbox

# Test connection
python3 -c "from openbox import AsyncWindowManager; import asyncio; \
    async def t(): async with AsyncWindowManager() as wm: \
    print(await wm.get_version()); asyncio.run(t())"

# Check D-Bus permissions
dbus-send --session --dest=org.openbox.WindowManager \
    --type=method_call /org/openbox/WindowManager \
    org.openbox.WindowManager.GetVersion
```

### 5. Extensions Not Loading

**Symptoms**: Extensions don't appear in `obctl extension list`.

**Solutions**:

```bash
# Check extension directory
ls -la ~/.config/openbox/extensions/

# Verify extension is valid
file ~/.config/openbox/extensions/*.so

# Check dependencies
ldd ~/.config/openbox/extensions/myext.so

# Enable debug logging
openbox --debug 2>&1 | grep -i extension

# Check for errors (4.0.1)
obctl extension list --verbose
```

### 6. High CPU Usage

**Symptoms**: Openbox using excessive CPU.

**Solutions**:

**4.0.1 Fix**: This is fixed in 4.0.1. If still experiencing issues:

```bash
# Check which process is using CPU
top -p $(pgrep -d',' openbox)

# Profile Openbox (if built with profiling)
openbox --profile 2>&1 | head -100

# Check for event loops
dbus-monitor --session "interface='org.openbox.WindowManager'" 2>&1 | head

# Disable extensions temporarily
mv ~/.config/openbox/extensions ~/.config/openbox/extensions.bak
```

### 7. Memory Leaks

**Symptoms**: Openbox memory usage grows over time.

**Solutions**:

**4.0.1 Fix**: Major memory leaks fixed in 4.0.1. If still experiencing:

```bash
# Monitor memory usage
watch -n 5 'ps -o pid,vsz,rss,comm -p $(pgrep openbox)'

# Check for leaked windows
obctl window list | wc -l

# Test with Valgrind (slow)
valgrind --leak-check=full openbox --replace 2>&1 | tee /tmp/valgrind.log
```

### 8. D-Bus Connection Lost

**Symptoms**: "Connection refused" or "Service unknown" errors.

**Solutions**:

**4.0.1**: Automatic reconnection should handle this. If not:

```bash
# Check D-Bus session
echo $DBUS_SESSION_BUS_ADDRESS

# Restart D-Bus session
dbus-run-session -- openbox

# Check if service is registered
dbus-send --session --dest=org.freedesktop.DBus \
    --type=method_call --print-reply /org/freedesktop/DBus \
    org.freedesktop.DBus.ListNames | grep openbox

# Restart Openbox D-Bus service
systemctl --user restart openbox.service
```

### 9. CSS Theme Parse Errors

**Symptoms**: Theme not applying, errors in logs.

**Solutions**:

**4.0.1**: Parser is more robust. Check for:

```bash
# Validate CSS
obctl theme validate

# Check for common errors
# - Unclosed braces
# - Invalid color values
# - Missing semicolons

# Test with minimal CSS
cat > /tmp/test-theme.css << 'EOF'
@theme-name "Test";

window {
  background: #333;
  color: #fff;
}
EOF

obctl theme set /tmp/test-theme.css
```

### 10. Window Focus Issues

**Symptoms**: Wrong window focused, focus not following mouse.

**Solutions**:

```bash
# Check focus model in config
grep -A5 '<focus>' ~/.config/openbox/rc.xml

# Test focus
obctl window focus 0x123456  # Replace with actual window ID

# Check for focus stealing prevention
grep -i focus ~/.config/openbox/rc.xml
```

## Performance Tuning

### Reduce Startup Time

```bash
# Disable unnecessary startup programs
# ~/.config/openbox/autostart
# Comment out: # nm-applet &

# Use lighter alternatives
# Replace: nitrogen with feh
# Replace: volumeicon with volumeicon-alsa
```

### Reduce Memory Usage

```bash
# Limit cache size (4.0.1)
# Set in ~/.config/openbox/environment:
# OPENBOX_CACHE_SIZE=100

# Disable unused extensions
rm ~/.config/openbox/extensions/unused.so
```

### Improve Responsiveness

```bash
# 4.0.1: Event batching is automatic
# For older versions, reduce event polling:

# ~/.config/openbox/environment
OPENBOX_EVENT_INTERVAL=16  # milliseconds (60fps)
```

## Debug Mode

Enable full debugging:

```bash
# Start with debug output
openbox --debug 2>&1 | tee /tmp/openbox-debug.log

# Enable specific debug categories
OPENBOX_DEBUG=event,window,theme openbox --replace

# Python bindings debug
PYTHONASYNCIODEBUG=1 python3 my-script.py
```

## Log Locations

| Log File | Description |
|----------|-------------|
| `~/.cache/openbox/openbox.log` | Main log |
| `~/.cache/openbox/theme-errors.log` | Theme parsing errors (4.0.1) |
| `~/.cache/openbox/extension-errors.log` | Extension errors (4.0.1) |
| `/tmp/openbox-debug.log` | Debug output |
| `journalctl --user -u openbox` | systemd service logs |

## Getting More Help

1. **Check documentation**: https://openbox.org/docs
2. **Search issues**: https://github.com/openbox/openbox/issues
3. **Ask community**: https://discord.gg/openbox
4. **Report bug**: Include `openbox --version` and debug logs

## 4.0.1 Specific Fixes

If upgrading from 4.0.0:

- Memory leaks fixed - no more restart needed
- D-Bus auto-reconnect - no more "connection refused"
- CSS parser robustness - fewer theme crashes
- Python async API - better for scripting
- Extension hot-reload - easier development

Run `openbox --version` to confirm you have 4.0.1 or higher.
