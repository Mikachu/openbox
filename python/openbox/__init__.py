\"\"\"Openbox Python bindings for automation and testing.

This module provides Pythonic access to the Openbox window manager
via D-Bus and direct C API bindings where available.

Example:
    >>> import openbox
    >>> wm = openbox.WindowManager()
    >>> wm.focus_window(0x123456)
    >>> for window in wm.get_windows():
    ...     print(f\"{window.id}: {window.title}\")
\"\"\"

__version__ = \"4.0.0\"
__all__ = [
    \"WindowManager\",
    \"Window\", 
    \"Desktop\",
    \"Theme\",
    \"Config\",
    \"errors\",
]

from .core import WindowManager, Window, Desktop
from .theme import Theme
from .config import Config
from . import errors

# Convenience imports
from .core import get_wm as get_window_manager
