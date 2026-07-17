\"\"\"Openbox Python bindings - STABILITY IMPROVED v4.0.1

Improvements:
- AsyncWindowManager as primary interface\n- Better error handling with specific exceptions\n- Complete type stubs
- Connection retry logic
- Signal subscription support

Example:
    >>> import asyncio
    >>> from openbox import AsyncWindowManager
    
    >>> async def main():
    ...     async with AsyncWindowManager() as wm:
    ...         windows = await wm.get_windows()
    ...         for w in windows:
    ...             print(f\"{w.id}: {w.title}\")
    
    >>> asyncio.run(main())
\"\"\"

__version__ = \"4.0.1\"
__all__ = [
    # Primary async interface (recommended)
    \"AsyncWindowManager\",
    \"get_wm\",
    \n    # Legacy sync interface (deprecated)\n    \"WindowManager\",
    \n    # Data types\n    \"Window\",
    \"Desktop\", 
n    \"Monitor\",\n    \"Geometry\",\n    \"WindowState\",\n    \"WindowType\",\n    \n    # Exceptions\n    \"OpenboxError\",\n    \"ConnectionError\",\n    \"WindowNotFoundError\",\n    \"DesktopNotFoundError\",\n    \"PermissionError\",\n    \"ConfigError\",\n    \"DBusError\",\n    \"TimeoutError\",\n    \"ValidationError\",\n]

from .core import (
    AsyncWindowManager,\n    WindowManager,  # Deprecated, use AsyncWindowManager\n    get_wm,\n    Window,\n    Desktop,\n    Monitor,\n    Geometry,\n    WindowState,\n    WindowType,\n)

from .errors import (
n    OpenboxError,\n    ConnectionError,\n    WindowNotFoundError,\n    DesktopNotFoundError,\n    PermissionError,\n    ConfigError,\n    DBusError,\n    TimeoutError,\n    ValidationError,\n)

# Backwards compatibility - warn about deprecated usage
import warnings

def _deprecation_warning():\n    warnings.warn(\n        \"Importing WindowManager directly is deprecated. \"\n        \"Use AsyncWindowManager instead for better performance and reliability.\",\n        DeprecationWarning,\n        stacklevel=3\n    )

# Set up deprecation warning for old imports\nif 'WindowManager' in dir():\n    _original_wm = WindowManager\n    \n    class _DeprecatedWindowManager(_original_wm):\n        def __init__(self, *args, **kwargs):\n            _deprecation_warning()\n            super().__init__(*args, **kwargs)\n    \n    # Replace with deprecated wrapper\n    WindowManager = _DeprecatedWindowManager\n