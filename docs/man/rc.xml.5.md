---
title: rc.xml
section: 5
header: Openbox Configuration
footer: Openbox 4.0
---

# NAME

rc.xml - Openbox configuration file

# SYNOPSIS

$XDG_CONFIG_HOME/openbox/rc.xml

# DESCRIPTION

The rc.xml file is the main configuration file for Openbox 4. It controls key bindings, mouse bindings, desktops, themes, and general window manager behavior.

# FILE FORMAT

The configuration file uses XML format with the following structure:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<openbox_config xmlns="http://openbox.org/4.0/rc">
  <theme>
    <name>Nightfall</name>
  </theme>
  <desktops>
    <number>4</number>
  </desktops>
  <keyboard>
    <keybind key="A-F4">
      <action name="Close"/>
    </keybind>
  </keyboard>
</openbox_config>
```

# SECTIONS

## theme

Controls the visual appearance of window decorations.

- **name** - Theme name to use
- **variant** - Theme variant: "light", "dark", or "auto"

Example:
```xml
<theme>
  <name>Nightfall</name>
  <variant>dark</variant>
</theme>
```

## desktops

Configures virtual desktops.

- **number** - Number of desktops (1-99)
- **names** - Optional desktop names
- **popupTime** - Desktop switch popup duration in ms (0 to disable)

Example:
```xml
<desktops>
  <number>4</number>
  <names>
    <name>Web</name>
    <name>Code</name>
    <name>Chat</name>
    <name>Misc</name>
  </names>
  <popupTime>875</popupTime>
</desktops>
```

## keyboard

Defines keyboard shortcuts.

### keybind

- **key** - Key combination (see KEY SYNTAX below)

### action

Available actions:
- **Close** - Close window
- **Focus** - Focus window
- **Raise** - Raise window
- **Lower** - Lower window
- **Maximize** - Maximize window
- **Minimize** - Minimize window
- **Move** - Start move operation
- **Resize** - Start resize operation
- **ShowMenu** - Show context menu
- **NextWindow** - Switch to next window
- **PreviousWindow** - Switch to previous window
- **Desktop** - Switch desktop
- **Execute** - Run command

Example:
```xml
<keyboard>
  <keybind key="A-F4">
    <action name="Close"/>
  </keybind>
  <keybind key="A-Return">
    <action name="Execute">
      <command>terminal</command>
    </action>
  </keybind>
</keyboard>
```

## mouse

Defines mouse bindings.

### context

Where the binding applies:
- **Frame** - Window decorations
- **Client** - Window content
- **Desktop** - Root window
- **Titlebar** - Title bar only
- **Close** - Close button
- **Maximize** - Maximize button
- **Minimize** - Minimize button

### mousebind

- **button** - Button: "Left", "Middle", "Right", or "Up"/"Down" for scroll
- **action** - Press, Click, DoubleClick, Drag, or Motion

Example:
```xml
<mouse>
  <context name="Frame">
    <mousebind button="Left" action="Press">
      <action name="Focus"/>
      <action name="Raise"/>
    </mousebind>
  </context>
</mouse>
```

## margins

Screen edge margins for tiling or other purposes.

- **top**, **bottom**, **left**, **right** - Pixel values

Example:
```xml
<margins>
  <top>0</top>
  <bottom>0</bottom>
  <left>0</left>
  <right>0</right>
</margins>
```

## menu

Menu configuration.

- **hideDelay** - Delay before hiding submenus (ms)
- **middle** - Middle-click to close window
- **submenuShowDelay** - Delay before showing submenus (ms)

Example:
```xml
<menu>
  <hideDelay>250</hideDelay>
  <middle>no</middle>
  <submenuShowDelay>100</submenuShowDelay>
</menu>
```

## applications

Application-specific settings.

### application

- **name** - Window class or name (supports wildcards)
- **role** - Window role
- **title** - Window title

Properties:
- **decor** - Show decorations: yes/no
- **shade** - Start shaded: yes/no
- **position** - Initial position
- **size** - Initial size
- **focus** - Start focused: yes/no
- **desktop** - Desktop to place on
- **layer** - Stacking layer: Normal, Above, Below

Example:
```xml
<applications>
  <application name="firefox">
    <decor>yes</decor>
    <maximized>yes</maximized>
  </application>
</applications>
```

# KEY SYNTAX

Key combinations use the following modifiers:

- **S** - Shift
- **C** - Control
- **A** - Alt
- **W** - Super (Windows key)
- **M** - Meta

Combine with key names: "A-F4", "C-A-t", "W-Tab"

Special keys: Escape, Return, Tab, space, Print, Pause, Home, End, etc.

Function keys: F1 through F35.

# FILES

- $XDG_CONFIG_HOME/openbox/rc.xml - User configuration
- /etc/xdg/openbox/rc.xml - System defaults

# SEE ALSO

**openbox**(1), **obctl**(1), **menu.xml**(5)

# AUTHORS

Openbox Development Team

# COPYRIGHT

Copyright (c) 2002-2024 Openbox Development Team.
Licensed under GPL-2.0-or-later.
