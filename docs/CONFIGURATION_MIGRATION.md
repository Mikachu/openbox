# Openbox Configuration Migration Guide

## Overview

Openbox 4 introduces a modern JSON-based configuration format alongside the traditional XML format. This guide explains the new format, how to migrate existing configurations, and the benefits of the new system.

## Why JSON?

- **Better tooling**: JSON Schema provides IDE autocomplete and validation
- **Hot reload**: Changes are applied instantly without restart
- **Clearer syntax**: Less verbose than XML
- **Modern libraries**: Native support in most programming languages
- **Error messages**: Detailed validation errors with line numbers

## Quick Start

### New Installations

JSON configuration is used by default for new installations:

```bash
# Creates ~/.config/openbox/rc.json
openbox --reconfigure
```

### Migrating Existing Configurations

```bash
# Convert rc.xml to rc.json
openbox-xml2json ~/.config/openbox/rc.xml

# Convert menu.xml to menu.json
openbox-xml2json ~/.config/openbox/menu.xml

# Or convert both with automatic backup
openbox-xml2json ~/.config/openbox/rc.xml -o ~/.config/openbox/
openbox-xml2json ~/.config/openbox/menu.xml -o ~/.config/openbox/
```

## Configuration File Locations

Openbox searches for configuration files in this order:

1. `~/.config/openbox/rc.json` (user JSON)
2. `~/.config/openbox/rc.xml` (user XML)
3. `/etc/xdg/openbox/rc.json` (system JSON)
4. `/etc/xdg/openbox/rc.xml` (system XML)

## Format Comparison

### Theme Configuration

**XML (rc.xml)**:
```xml
<theme>
  <name>Clearlooks</name>
  <titleLayout>NLIMC</titleLayout>
  <keepBorder>yes</keepBorder>
  <animateIconify>yes</animateIconify>
  <font place="ActiveWindow">
    <name>sans</name>
    <size>8</size>
    <weight>bold</weight>
    <slant>normal</slant>
  </font>
</theme>
```

**JSON (rc.json)**:
```json
{
  "theme": {
    "name": "Clearlooks",
    "titleLayout": "NLIMC",
    "keepBorder": true,
    "animateIconify": true,
    "fonts": {
      "activeWindow": {
        "name": "sans",
        "size": 8,
        "weight": "bold",
        "slant": "normal"
      }
    }
  }
}
```

### Key Bindings

**XML**:
```xml
<keybind key="W-F1">
  <action name="GoToDesktop">
    <to>1</to>
  </action>
</keybind>
<keybind key="A-F4">
  <action name="Close"/>
</keybind>
```

**JSON**:
```json
{
  "keybinds": [
    {
      "key": "W-F1",
      "action": {
        "name": "GoToDesktop",
        "to": 1
      }
    },
    {
      "key": "A-F4",
      "action": { "name": "Close" }
    }
  ]
}
```

### Mouse Bindings

**XML**:
```xml
<mousebind button="A-Left" action="Drag">
  <action name="Move"/>
</mousebind>
```

**JSON**:
```json
{
  "bindings": [
    {
      "context": "frame",
      "button": "A-Left",
      "event": "drag",
      "action": { "name": "Move" }
    }
  ]
}
```

## JSON Schema Validation

All configuration files are validated against JSON Schema. This provides:

- **Autocomplete** in supported editors (VS Code, Vim with coc-json, etc.)
- **Validation** of values (e.g., checking that `focus_delay` is a positive integer)
- **Documentation** via schema descriptions

### Schema Files

- `rc-schema.json` - Main configuration schema
- `menu-schema.json` - Menu configuration schema

### IDE Setup

#### VS Code

Add to `.vscode/settings.json`:

```json
{
  "json.schemas": [
    {
      "fileMatch": ["rc.json"],
      "url": "https://openbox.org/schemas/rc-4.0.json"
    },
    {
      "fileMatch": ["menu.json"],
      "url": "https://openbox.org/schemas/menu-4.0.json"
    }
  ]
}
```

#### Vim/Neovim with coc.nvim

Add to `coc-settings.json`:

```json
{
  "json.schemas": [
    {
      "fileMatch": "rc.json",
      "url": "https://openbox.org/schemas/rc-4.0.json"
    }
  ]
}
```

## Hot Reload

JSON configurations support hot reloading:

```bash
# Enable hot reload (default)
openbox --hot-reload

# Disable hot reload
openbox --no-hot-reload
```

When enabled, changes to `rc.json` are automatically applied without restarting Openbox. Errors are reported via desktop notifications.

## Migration Tool Reference

### `openbox-xml2json`

Converts XML configuration to JSON format.

**Usage**:
```bash
openbox-xml2json [OPTIONS] <input.xml>
```

**Options**:
- `-o, --output DIR` - Output directory (default: same as input)
- `-n, --dry-run` - Show what would be done without writing files
- `-v, --verbose` - Verbose output
- `--no-validate` - Skip JSON Schema validation
- `--no-backup` - Don't create .xml.backup files
- `-h, --help` - Show help
- `--version` - Show version

**Examples**:

```bash
# Convert with automatic backup
openbox-xml2json ~/.config/openbox/rc.xml

# Convert to specific directory
openbox-xml2json ~/.config/openbox/rc.xml -o ~/converted/

# Preview conversion without writing
openbox-xml2json --dry-run -v ~/.config/openbox/menu.xml
```

## Environment Variables

- `OPENBOX_CONFIG_FORMAT=json|xml` - Force specific format
- `OPENBOX_CONFIG_PATH` - Override config file location
- `OPENBOX_NO_HOT_RELOAD=1` - Disable hot reload

## Troubleshooting

### Validation Errors

If your JSON fails validation:

```bash
# Check with verbose output
openbox-xml2json -v ~/.config/openbox/rc.xml

# Validate existing JSON
openbox --validate-config ~/.config/openbox/rc.json
```

### Migration Failures

Common issues and solutions:

| Issue | Solution |
|-------|----------|
| "Unknown action name" | Check action name spelling against schema |
| "Invalid key combination" | Use format like `C-A-Left` (Ctrl+Alt+Left) |
| "Missing required field" | Check all required fields in schema |

### Backward Compatibility

XML configurations continue to work. To force XML mode:

```bash
OPENBOX_CONFIG_FORMAT=xml openbox
```

## Advanced Topics

### Programmatic Configuration

Generate configuration programmatically:

```python
import json

config = {
    "version": "4.0.0",
    "theme": {
        "name": "Generated",
        "fonts": {
            "activeWindow": {
                "name": "Monospace",
                "size": 10
            }
        }
    },
    "keybinds": [
        {
            "key": "W-Return",
            "action": {
                "name": "Execute",
                "command": "terminal"
            }
        }
    ]
}

with open('rc.json', 'w') as f:
    json.dump(config, f, indent=2)
```

### Custom Schemas

Extend the schema for custom actions:

```json
{
  "$schema": "https://openbox.org/schemas/rc-4.0.json",
  "customActions": {
    "myAction": {
      "command": "custom-script",
      "args": ["--flag"]
    }
  }
}
```

## Migration Checklist

- [ ] Backup existing configuration (`cp -r ~/.config/openbox ~/openbox-backup`)
- [ ] Run migration tool (`openbox-xml2json rc.xml`)
- [ ] Verify JSON is valid (`openbox --validate-config rc.json`)
- [ ] Test configuration in new session
- [ ] Update any scripts that modify XML directly
- [ ] Set up IDE integration for JSON editing
- [ ] Enable hot reload for development

## Support

For migration issues:

1. Check the [Openbox Wiki](https://openbox.org/wiki/)
2. Run with verbose logging: `openbox -v`
3. Validate your JSON: `openbox --validate-config`
4. Report issues at: https://github.com/openbox/openbox/issues
