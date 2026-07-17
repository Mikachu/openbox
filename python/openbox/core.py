\"\"\"Core Openbox Python bindings.\"\"\"

import dbus
import dbus.service
from typing import List, Optional, Dict, Any, Tuple, Callable
from dataclasses import dataclass
from enum import IntFlag
import weakref

from .errors import OpenboxError, WindowNotFoundError, ConnectionError


class WindowState(IntFlag):
    \"\"\"Window state flags.\"\"\"
    MODAL = 1 << 0
    STICKY = 1 << 1
    MAXIMIZED_VERT = 1 << 2
    MAXIMIZED_HORZ = 1 << 3
    SHADED = 1 << 4
    SKIP_TASKBAR = 1 << 5
    SKIP_PAGER = 1 << 6
    HIDDEN = 1 << 7
    FULLSCREEN = 1 << 8
    ABOVE = 1 << 9
    BELOW = 1 << 10
    DEMANDS_ATTENTION = 1 << 11


@dataclass
class Window:
    \"\"\"Represents an Openbox managed window.\"\"\"
    id: int
    title: str
    wm_class: str
    wm_instance: str
    desktop: int
    states: WindowState
    x: int
    y: int
    width: int
    height: int
    
    def __repr__(self) -> str:
        return f\"Window({self.id:#x}, {self.title!r})\"
    
    @property
    def geometry(self) -> Tuple[int, int, int, int]:
        \"\"\"Returns (x, y, width, height).\"\"\"
        return (self.x, self.y, self.width, self.height)


@dataclass
class Desktop:
    \"\"\"Represents a virtual desktop.\"\"\"
    number: int
    name: str
    is_current: bool
    
    def __repr__(self) -> str:
        return f\"Desktop({self.number}, {self.name!r})\"


class WindowManager:
    \"\"\"Main interface to Openbox window manager.
    
    This class provides methods to control and query the Openbox
    window manager via D-Bus.
    
    Example:
        >>> wm = WindowManager()
        >>> wm.get_version()
        '4.0.0'
        >>> current = wm.get_active_window()
        >>> if current:
        ...     print(f\"Active: {current.title}\")
    \"\"\" 
    
    _instance = None
    
    def __new__(cls, *args, **kwargs):
        \"\"\"Singleton pattern - only one WM connection.\"\"\"
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls._instance._initialized = False
        return cls._instance
    
    def __init__(self, bus_address: Optional[str] = None):
        \"\"\"Initialize connection to Openbox.
        
        Args:
            bus_address: Optional D-Bus address. Uses session bus if not provided.
            
        Raises:
            ConnectionError: If cannot connect to Openbox.
        \"\"\"
        if self._initialized:
            return
            
        try:
            if bus_address:
                self._bus = dbus.bus.BusConnection(bus_address)
            else:
                self._bus = dbus.SessionBus()
                
            self._proxy = self._bus.get_object(
                'org.openbox.WindowManager',
                '/org/openbox/WindowManager'
            )
            self._interface = dbus.Interface(
                self._proxy,
                'org.openbox.WindowManager'
            )
            self._config = dbus.Interface(
                self._proxy,
                'org.openbox.Config'
            )
            self._initialized = True
            
        except dbus.DBusException as e:
            raise ConnectionError(
                f\"Failed to connect to Openbox: {e}. "
                "Is Openbox running?"
            ) from e
    
    def get_version(self) -> str:
        \"\"\"Get Openbox version string.\"\"\"
        return self._interface.GetVersion()
    
    def get_windows(self) -> List[Window]:
        \"\"\"Get list of all managed windows.\"\"\"
        raw_windows = self._interface.GetWindows()
        windows = []
        for w in raw_windows:
            windows.append(Window(
                id=int(w[0]),
                title=str(w[1].get('title', '')),
                wm_class=str(w[1].get('class', '')),
                wm_instance=str(w[1].get('instance', '')),
                desktop=int(w[1].get('desktop', 0)),
                states=WindowState(int(w[1].get('states', 0))),
                x=int(w[1].get('x', 0)),
                y=int(w[1].get('y', 0)),
                width=int(w[1].get('width', 0)),
                height=int(w[1].get('height', 0))
            ))
        return windows
    
    def get_active_window(self) -> Optional[Window]:
        \"\"\"Get currently focused window.\"\"\"
        try:
            window_id = self._interface.GetActiveWindow()
            if window_id == 0:
                return None
            for w in self.get_windows():
                if w.id == window_id:
                    return w
            return None
        except dbus.DBusException:
            return None
    
    def get_window_by_id(self, window_id: int) -> Optional[Window]:
        \"\"\"Get window by its ID.
        
        Args:
            window_id: The window ID to look up.
            
        Returns:
            Window if found, None otherwise.
        \"\"\"
        for w in self.get_windows():
            if w.id == window_id:
                return w
        return None
    
    def get_window_by_title(self, title: str) -> Optional[Window]:
        \"\"\"Find window by title (substring match).\"\"\"
        for w in self.get_windows():
            if title.lower() in w.title.lower():
                return w
        return None
    
    def focus_window(self, window: Window) -> None:
        \"\"\"Focus a specific window.\"\"\"
        self._interface.FocusWindow(dbus.UInt32(window.id))
    
    def close_window(self, window: Window) -> None:
        \"\"\"Close a window.\"\"\"
        self._interface.CloseWindow(dbus.UInt32(window.id))
    
    def maximize_window(self, window: Window, 
                       horizontal: bool = True,
                       vertical: bool = True) -> None:
        \"\"\"Maximize a window.\"\"\"
        self._interface.MaximizeWindow(
            dbus.UInt32(window.id),
            horizontal,
            vertical
        )
    
    def minimize_window(self, window: Window) -> None:
        \"\"\"Minimize (iconify) a window.\"\"\"
        self._interface.MinimizeWindow(dbus.UInt32(window.id))
    
    def move_window(self, window: Window, x: int, y: int) -> None:
        \"\"\"Move window to position.\"\"\"
        self._interface.MoveWindow(dbus.UInt32(window.id), x, y)
    
    def resize_window(self, window: Window, width: int, height: int) -> None:
        \"\"\"Resize window.\"\"\"
        self._interface.ResizeWindow(
            dbus.UInt32(window.id), 
            dbus.UInt32(width), 
            dbus.UInt32(height)
        )
    
    def get_desktops(self) -> List[Desktop]:
        \"\"\"Get list of all desktops.\"\"\"
        current = self._interface.GetCurrentDesktop()
        raw_desktops = self._interface.GetDesktops()
        desktops = []
        for i, name in enumerate(raw_desktops):
            desktops.append(Desktop(
                number=i,
                name=str(name),
                is_current=(i == current)
            ))
        return desktops
    
    def get_current_desktop(self) -> int:
        \"\"\"Get current desktop number.\"\"\"
        return self._interface.GetCurrentDesktop()
    
    def switch_desktop(self, desktop_number: int) -> None:
        \"\"\"Switch to a specific desktop.\"\"\"
        self._interface.SwitchDesktop(dbus.UInt32(desktop_number))
    
    def send_window_to_desktop(self, window: Window, desktop: int) -> None:
        \"\"\"Move window to another desktop.\"\"\"
        self._interface.MoveWindowToDesktop(
            dbus.UInt32(window.id),
            dbus.UInt32(desktop)
        )
    
    def reconfigure(self) -> None:
        \"\"\"Reload configuration from disk.\"\"\"
        self._interface.Reconfigure()
    
    def restart(self) -> None:
        \"\"\"Restart Openbox.\"\"\"
        self._interface.Restart()
    
    def exit(self) -> None:
        \"\"\"Exit Openbox.\"\"\"
        self._interface.Exit()
    
    def execute(self, command: str) -> None:
        \"\"\"Execute a shell command.\"\"\"
        self._interface.Execute(command)
    
    def show_menu(self, menu_id: str) -> None:
        \"\"\"Show a menu by ID.\"\"\"
        self._interface.ShowMenu(menu_id)
    
    def get_config(self, key: str) -> Any:
        \"\"\"Get configuration value.\"\"\"
        return self._config.Get(key)
    
    def set_config(self, key: str, value: Any) -> None:
        \"\"\"Set configuration value.\"\"\"
        self._config.Set(key, value)
    
    def on_window_event(self, callback: Callable[[str, Window], None]) -> None:
        \"\"\"Register callback for window events.
        
        Events: 'created', 'destroyed', 'focused', 'unfocused', 
                'mapped', 'unmapped', 'configured'
        \"\"\"
        # TODO: Implement D-Bus signal subscription
        pass
    
    def __enter__(self):
        return self
    
    def __exit__(self, *args):
        # Connection cleanup if needed
        pass


def get_wm() -> WindowManager:
    \"\"\"Get the global WindowManager instance.\"\"\"
    return WindowManager()
