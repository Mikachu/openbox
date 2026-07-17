---
title: openbox
section: 1
header: Openbox Manual
footer: Openbox 4.0
---

n# NAME

nopenbox - A highly configurable, next-generation stacking window manager

n# SYNOPSIS

n**openbox** \[*OPTIONS*\]

n# DESCRIPTION

n**Openbox** is a lightweight window manager for the X11 and Wayland windowing systems. It provides a fast, modern, and highly configurable environment for managing application windows.

nOpenbox 4 features a complete rewrite of the core in Rust, providing memory safety and improved performance while maintaining compatibility with existing configurations.

n# OPTIONS

n**\\-\\-help**
n:   Display help message and exit.

n**\\-v**, **\\-\\-version**
n:   Display version information and exit.

n**\\-\\-replace**
n:   Replace the currently running window manager. This is the recommended way to switch to Openbox from another window manager.

n**\\-\\-config-file** *FILE*
n:   Specify an alternative configuration file. Default is *\\$XDG_CONFIG_HOME/openbox/rc.xml*.

n**\\-\\-config-test**
n:   Test the configuration file for errors and exit. Returns 0 on success, non-zero on failure.

n**\\-\\-debug**
n:   Enable debug output. Useful for troubleshooting.

n**\\-\\-sync**
n:   Run in synchronous mode (for debugging X11 errors).

n**\\-\\-startup** *COMMAND*
n:   Run *COMMAND* after initialization. Useful for starting panels or other accessories.

n**\\-\\-exit**
n:   Exit Openbox gracefully (send D-Bus signal to running instance).

n**\\-\\-reconfigure**
n:   Reload configuration (send D-Bus signal to running instance).

n**\\-\\-restart**
n:   Restart Openbox (send D-Bus signal to running instance).

n# FILES

n*\\$XDG_CONFIG_HOME/openbox/rc.xml*
n:   Main configuration file. See **rc.xml**(5).

n*\\$XDG_CONFIG_HOME/openbox/menu.xml*
n:   Menu configuration. See **menu.xml**(5).

n*\\$XDG_CONFIG_HOME/openbox/environment*
n:   Environment variable settings.

n*\\$XDG_CONFIG_HOME/openbox/autostart*
n:   Autostart script.

n*\\$XDG_DATA_HOME/themes/*
n:   Theme directory.

n# ENVIRONMENT

n**DISPLAY**
n:   The X11 display to use.

n**XDG_CONFIG_HOME**
n:   Base directory for configuration. Defaults to *\\$HOME/.config*.

n**XDG_DATA_HOME**
n:   Base directory for data files. Defaults to *\\$HOME/.local/share*.

n**OPENBOX_DEBUG**
n:   Set to enable additional debug output.

n# EXAMPLES

nStart Openbox, replacing the current window manager:

n:   **openbox \\-\\-replace**

nTest a configuration file:

n:   **openbox \\-\\-config-file \\$HOME/.config/openbox/test.xml \\-\\-config-test**

nStart with a custom startup command:

n:   **openbox \\-\\-startup \"tint2 \\&\"**

n# SIGNALS

nOpenbox 4 uses D-Bus for IPC. Legacy signals are supported for backward compatibility:

n**SIGUSR1**
n:   Reconfigure (equivalent to **\\-\\-reconfigure**).

n**SIGUSR2**
n:   Restart (equivalent to **\\-\\-restart**).

n**SIGTERM**
n:   Exit gracefully (equivalent to **\\-\\-exit**).

n# SEE ALSO

n**obctl**(1), **openbox-session**(1), **rc.xml**(5), **menu.xml**(5)

nThe Openbox documentation at <https://openbox.org/documentation>

n# AUTHORS

nOpenbox 4 was written by the Openbox Development Team.

nOpenbox 3.x was originally written by Mikael Magnusson and Dana Jansens, based on Blackbox by Brad Hughes.

n# COPYRIGHT

nCopyright \\(co 2002-2024 Openbox Development Team.

nThis is free software; see the source for copying conditions. There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
