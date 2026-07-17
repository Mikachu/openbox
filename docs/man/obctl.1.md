---
title: obctl
section: 1
header: Openbox Manual
footer: Openbox 4.0
---

n# NAME

nobctl - Openbox control utility

n# SYNOPSIS

n**obctl** \[*GLOBAL_OPTIONS*\] *COMMAND* \[*ARGS*\]...

n**obctl** \[\\-\\-help | \\-v | \\-\\-version\]

n# DESCRIPTION

n**obctl** is the command-line control utility for Openbox 4. It communicates with the running Openbox instance via D-Bus to control window management, query state, and manage configuration.

n# GLOBAL OPTIONS

n**\\-\\-help**
n:   Display help message and exit.

n**\\-v**, **\\-\\-version**
n:   Display version information and exit.

n**\\-\\-bus** *ADDRESS*
n:   Use specific D-Bus session bus address.

n**\\-\\-json**
n:   Output in JSON format for scripting.

n**\\-\\-quiet**, **\\-q**
n:   Suppress non-error output.

n# COMMANDS

n## Lifecycle

n**lifecycle reconfigure**
n:   Reload configuration from disk.

n**lifecycle restart**
n:   Restart Openbox (preserves session).

n**lifecycle exit**
n:   Exit Openbox gracefully.

n**lifecycle status**
n:   Show Openbox status (running, version, etc.).

n## Window Management

n**window list**
n:   List all managed windows.

n**window focus** *WINDOW_ID*
n:   Focus the specified window.

n**window close** *WINDOW_ID*
n:   Close the specified window.

n**window raise** *WINDOW_ID*
n:   Raise window to top of stack.

n**window lower** *WINDOW_ID*
n:   Lower window to bottom of stack.

n**window maximize** *WINDOW_ID* \[*DIRECTION*\]
n:   Maximize window. Direction: *horizontal*, *vertical*, or *both* (default).

n**window unmaximize** *WINDOW_ID*
n:   Unmaximize window.

n**window minimize** *WINDOW_ID*
n:   Minimize (iconify) window.

n**window shade** *WINDOW_ID*
n:   Roll up window to titlebar only.

n**window unshade** *WINDOW_ID*
n:   Unroll window.

n**window move** *WINDOW_ID* *X* *Y*
n:   Move window to coordinates.

n**window resize** *WINDOW_ID* *WIDTH* *HEIGHT*
n:   Resize window.

n**window desktop** *WINDOW_ID* *DESKTOP*
n:   Move window to desktop (0-indexed).

n**window pin** *WINDOW_ID*
n:   Pin window to all desktops.

n**window unpin** *WINDOW_ID*
n:   Unpin window.

n**window info** *WINDOW_ID*
n:   Show detailed window information.

n## Desktop Management

n**desktop list**
n:   List all desktops.

n**desktop current**
n:   Show current desktop number.

n**desktop switch** *DESKTOP*
n:   Switch to desktop (0-indexed).

n**desktop add** \[*NAME*\]
n:   Add a new desktop.

n**desktop remove** *DESKTOP*
n:   Remove a desktop.

n**desktop rename** *DESKTOP* *NAME*
n:   Rename a desktop.

n## Configuration

n**config get** *KEY*
n:   Get configuration value.

n**config set** *KEY* *VALUE*
n:   Set configuration value (requires restart for some values).

n**config validate** \[*FILE*\]
n:   Validate configuration file.

n**config path**
n:   Show configuration file path.

n## Theme

n**theme list**
n:   List available themes.

n**theme current**
n:   Show current theme name.

n**theme set** *THEME*
n:   Set active theme.

n**theme reload**
n:   Reload current theme.

n**theme path** *THEME*
n:   Show theme directory path.

n## Menus

n**menu show** *MENU_ID*
n:   Show menu by ID.

n**menu list**
n:   List available menus.

n**menu reload**
n:   Reload menu configuration.

n# EXAMPLES

nList all windows:

n:   **obctl window list**

nFocus window by ID:

n:   **obctl window focus 0x1234567**

nSwitch to desktop 3:

n:   **obctl desktop switch 2**

nGet JSON output for scripting:

n:   **obctl \\-\\-json window list | jq '.[].title'**

nReload configuration:

n:   **obctl lifecycle reconfigure**

nSet dark theme:

n:   **obctl theme set Nightfall**

n# EXIT STATUS

n**0**
n:   Success.

n**1**
n:   General error.

n**2**
n:   D-Bus connection failed (is Openbox running?).

n**3**
n:   Invalid arguments.

n**4**
n:   Command not found.

n# ENVIRONMENT

n**DBUS_SESSION_BUS_ADDRESS**
n:   D-Bus session bus address.

n**XDG_CONFIG_HOME**
n:   Configuration directory.

n# FILES

n*\\$XDG_CONFIG_HOME/openbox/rc.xml*
n:   Configuration file.

n# NOTES

n**obctl** requires a running Openbox instance with D-Bus support enabled.

nFor legacy compatibility, the **openbox** command with **\\-\\-reconfigure**, **\\-\\-restart**, and **\\-\\-exit** options still work.

n# SEE ALSO

n**openbox**(1), **openbox-session**(1), **rc.xml**(5)

n# AUTHORS

nOpenbox Development Team.

n# COPYRIGHT

nCopyright \\(co 2024 Openbox Development Team.
