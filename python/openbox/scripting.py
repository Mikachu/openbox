\"\"\"Lua and JavaScript scripting support for Openbox.\"\"\"

import lupa
from lupa import LuaRuntime
import json
import os
from typing import Dict, Any, Callable, Optional
from pathlib import Path

from .core import WindowManager, Window, WindowState


class ScriptEngine:
    \"\"\"Embedded scripting engine for user customization.
    
    Supports both Lua and JavaScript (via QuickJS) for user scripts.
    \"\"\"
    
    def __init__(self, wm: WindowManager):
        self.wm = wm
        self._lua = None
        self._js = None
        self._hooks: Dict[str, list] = {}
        self._script_dir = Path.home() / \".config\" / \"openbox\" / \"scripts\"
        
    def _init_lua(self) -> LuaRuntime:
        \"\"\"Initialize Lua runtime with Openbox API.\"\"\"
        if self._lua is None:
            self._lua = LuaRuntime(unpack_returned_tuples=True)
            
            # Expose Openbox API to Lua
            lua_globals = self._lua.globals()
            lua_globals.openbox = {
                'get_version': self.wm.get_version,
                'get_windows': self._lua_windows,
                'get_active_window': self._lua_active_window,
                'focus_window': self._lua_focus,
                'close_window': self._lua_close,
                'move_window': self._lua_move,
                'resize_window': self._lua_resize,
                'get_desktops': self._lua_desktops,
                'switch_desktop': self.wm.switch_desktop,
                'execute': self.wm.execute,
                'reconfigure': self.wm.reconfigure,
                'log': self._lua_log,
            }
        return self._lua
    
    def _lua_windows(self):
        \"\"\"Get windows for Lua.\"\"\"
        return [
            {
                'id': w.id,
                'title': w.title,
                'class': w.wm_class,
                'desktop': w.desktop,
                'x': w.x,
                'y': w.y,
                'width': w.width,
                'height': w.height,
            }
            for w in self.wm.get_windows()
        ]
    
    def _lua_active_window(self):
        \"\"\"Get active window for Lua.\"\"\"
        w = self.wm.get_active_window()
        if w:
            return {
                'id': w.id,
                'title': w.title,
                'class': w.wm_class,
            }
        return None
    
    def _lua_focus(self, window_id: int):
        \"\"\"Focus window from Lua.\"\"\"
        w = self.wm.get_window_by_id(window_id)
        if w:
            self.wm.focus_window(w)
    
    def _lua_close(self, window_id: int):
        \"\"\"Close window from Lua.\"\"\"
        w = self.wm.get_window_by_id(window_id)
        if w:
            self.wm.close_window(w)
    
    def _lua_move(self, window_id: int, x: int, y: int):
        \"\"\"Move window from Lua.\"\"\"
        w = self.wm.get_window_by_id(window_id)
        if w:
            self.wm.move_window(w, x, y)
    
    def _lua_resize(self, window_id: int, width: int, height: int):
        \"\"\"Resize window from Lua.\"\"\"
        w = self.wm.get_window_by_id(window_id)
        if w:
            self.wm.resize_window(w, width, height)
    
    def _lua_desktops(self):
        \"\"\"Get desktops for Lua.\"\"\"
        return [
            {
                'number': d.number,
                'name': d.name,
                'current': d.is_current,
            }
            for d in self.wm.get_desktops()
        ]
    
    def _lua_log(self, level: str, message: str):
        \"\"\"Log from Lua.\"\"\"
        print(f\"[Lua {level}] {message}\")
    
    def run_lua(self, code: str) -> Any:
        \"\"\"Execute Lua code.\"\"\"
        lua = self._init_lua()
        return lua.execute(code)
    
    def run_lua_file(self, path: str) -> Any:
        \"\"\"Execute Lua file.\"\"\"
        with open(path, 'r') as f:
            return self.run_lua(f.read())
    
    def register_hook(self, event: str, callback: Callable):
        \"\"\"Register a Python callback for events.\"\"\"
        if event not in self._hooks:
            self._hooks[event] = []
        self._hooks[event].append(callback)
    
    def trigger_hook(self, event: str, *args, **kwargs):
        \"\"\"Trigger all callbacks for an event.\"\"\"
        for callback in self._hooks.get(event, []):
            try:
                callback(*args, **kwargs)
            except Exception as e:
                print(f\"Hook error: {e}\")


class WindowRules:
    \"\"\"Window matching and rule application.\"\"\"
    
    def __init__(self, wm: WindowManager):
        self.wm = wm
        self.rules: list = []
    
    def add_rule(self, match: Dict[str, Any], actions: Dict[str, Any]):
        \"\"\"Add a window rule.
        
        Args:
            match: Dict with keys like 'class', 'title', 'role', 'type'
            actions: Dict with actions like 'desktop', 'fullscreen', 'maximize'
        \"\"\"
        self.rules.append({'match': match, 'actions': actions})
    
    def match_window(self, window: Window) -> Optional[Dict]:
        \"\"\"Find matching rule for window.\"\"\"
        for rule in self.rules:
            if self._window_matches(window, rule['match']):
                return rule['actions']
        return None
    
    def _window_matches(self, window: Window, match: Dict) -> bool:
        \"\"\"Check if window matches criteria.\"\"\"
        if 'class' in match and match['class'] not in window.wm_class:
            return False
        if 'title' in match and match['title'] not in window.title:
            return False
        # Add more matchers...
        return True
    
    def apply_rules(self, window: Window):
        \"\"\"Apply matching rules to window.\"\"\"
        actions = self.match_window(window)
        if actions:
            if 'desktop' in actions:
                self.wm.send_window_to_desktop(window, actions['desktop'])
            if 'maximize' in actions and actions['maximize']:
                self.wm.maximize_window(window)
            if 'fullscreen' in actions and actions['fullscreen']:
                # wm.set_fullscreen not yet implemented
                pass
            if 'focus' in actions and actions['focus']:
                self.wm.focus_window(window)


class TilingManager:
    \"\"\"Simple tiling layout manager.\"\"\"
    
    LAYOUT_FLOATING = \"floating\"
    LAYOUT_TILED = \"tiled\"
    LAYOUT_MONOCLE = \"monocle\"
    
    def __init__(self, wm: WindowManager):
        self.wm = wm
        self.layouts: Dict[int, str] = {}  # desktop -> layout
        self.master_ratio = 0.5
    
    def set_layout(self, desktop: int, layout: str):
        \"\"\"Set layout for desktop.\"\"\"
        self.layouts[desktop] = layout
        self._apply_layout(desktop)
    
    def toggle_layout(self, desktop: int):
        \"\"\"Toggle between layouts.\"\"\"
        current = self.layouts.get(desktop, self.LAYOUT_FLOATING)
        layouts = [self.LAYOUT_FLOATING, self.LAYOUT_TILED, self.LAYOUT_MONOCLE]
        idx = layouts.index(current)
        next_idx = (idx + 1) % len(layouts)
        self.set_layout(desktop, layouts[next_idx])
    
    def _apply_layout(self, desktop: int):
        \"\"\"Apply current layout to desktop.\"\"\"
        layout = self.layouts.get(desktop, self.LAYOUT_FLOATING)
        windows = [w for w in self.wm.get_windows() if w.desktop == desktop]
        
        if layout == self.LAYOUT_FLOATING:
            return  # Do nothing
        
        elif layout == self.LAYOUT_TILED:
            self._tile_windows(windows)
        
        elif layout == self.LAYOUT_MONOCLE:
            self._monocle_windows(windows)
    
    def _tile_windows(self, windows: list):
        \"\"\"Apply tiled layout.\"\"\"
        if not windows:
            return
        
        # Simple master-stack layout
        screen_width = 1920  # TODO: Get actual screen size
        screen_height = 1080
        
        master_width = int(screen_width * self.master_ratio)
        stack_width = screen_width - master_width
        
        if windows:
            # Master window
            master = windows[0]
            self.wm.move_window(master, 0, 0)
            self.wm.resize_window(master, master_width, screen_height)
            
            # Stack windows
            stack_count = len(windows) - 1
            if stack_count > 0:
                stack_height = screen_height // stack_count
                for i, w in enumerate(windows[1:]):
                    self.wm.move_window(w, master_width, i * stack_height)
                    self.wm.resize_window(w, stack_width, stack_height)
    
    def _monocle_windows(self, windows: list):
        \"\"\"Apply monocle layout (fullscreen stack).\"\"\"
        # Just focus first, others are \"behind\"
        if windows:
            self.wm.focus_window(windows[0])
