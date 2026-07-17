#!/usr/bin/env python3
\"\"\"Command-line interface for Openbox Python bindings.

Provides obctl-like functionality in Python.
\"\"\"

import argparse
import sys
import json
from typing import Optional

from .core import WindowManager, WindowState
from .errors import OpenboxError, ConnectionError


def format_window(window, verbose: bool = False) -> str:
    \"\"\"Format window for display.\"\"\"
    if verbose:
        return f\"{window.id:#010x}  {window.desktop:2d}  {window.width:4d}x{window.height:4d}  {window.title[:50]}\"
    else:
        return f\"{window.id:#010x}  {window.title[:40]}\"


def cmd_list(wm: WindowManager, args) -> int:
    \"\"\"List windows.\"\"\"
    windows = wm.get_windows()
    
    if args.json:
        data = [
            {
                \"id\": w.id,
                \"title\": w.title,
                \"class\": w.wm_class,
                \"desktop\": w.desktop,
                \"x\": w.x,
                \"y\": w.y,
                \"width\": w.width,
                \"height\": w.height,
            }
            for w in windows
        ]
        print(json.dumps(data, indent=2))
    else:
        if args.verbose:
            print(f\"{'Window ID':12s}  {'Desk':4s}  {'Size':9s}  Title\")
            print(\"-\" * 70)
        for w in windows:
            print(format_window(w, args.verbose))
    
    return 0


def cmd_focus(wm: WindowManager, args) -> int:
    \"\"\"Focus a window.\"\"\"
    try:
        window_id = int(args.window, 0)  # Auto-detect base
        window = wm.get_window_by_id(window_id)
        if window:
            wm.focus_window(window)
            return 0
        else:
            print(f\"Window not found: {args.window}\", file=sys.stderr)
            return 1
    except ValueError:
        print(f\"Invalid window ID: {args.window}\", file=sys.stderr)
        return 1


def cmd_close(wm: WindowManager, args) -> int:
    \"\"\"Close a window.\"\"\"
    try:
        window_id = int(args.window, 0)
        window = wm.get_window_by_id(window_id)
        if window:
            wm.close_window(window)
            return 0
        else:
            print(f\"Window not found: {args.window}\", file=sys.stderr)
            return 1
    except ValueError:
        print(f\"Invalid window ID: {args.window}\", file=sys.stderr)
        return 1


def cmd_desktops(wm: WindowManager, args) -> int:
    \"\"\"List desktops.\"\"\"
    desktops = wm.get_desktops()
    current = wm.get_current_desktop()
    
    if args.json:
        data = [
            {
                \"number\": d.number,
                \"name\": d.name,
                \"current\": d.is_current,
            }
            for d in desktops
        ]
        print(json.dumps(data, indent=2))
    else:
        for d in desktops:
            marker = \"*\" if d.is_current else \" \"
            print(f\"{marker} {d.number}: {d.name}\")
    
    return 0


def cmd_switch(wm: WindowManager, args) -> int:
    \"\"\"Switch to desktop.\"\"\"
    try:
        desktop = int(args.desktop)
        wm.switch_desktop(desktop)
        return 0
    except ValueError:
        print(f\"Invalid desktop: {args.desktop}\", file=sys.stderr)
        return 1


def cmd_active(wm: WindowManager, args) -> int:
    \"\"\"Show active window.\"\"\"
    window = wm.get_active_window()
    if window:
        if args.json:
            data = {
                \"id\": window.id,
                \"title\": window.title,
                \"class\": window.wm_class,
                \"desktop\": window.desktop,
            }
            print(json.dumps(data, indent=2))
        else:
            print(f\"{window.id:#010x}  {window.title}\")
        return 0
    else:
        print(\"No active window\", file=sys.stderr)
        return 1


def cmd_execute(wm: WindowManager, args) -> int:
    \"\"\"Execute a command.\"\"\"
    wm.execute(\" \".join(args.command))
    return 0


def cmd_reconfigure(wm: WindowManager, args) -> int:
    \"\"\"Reconfigure Openbox.\"\"\"
    wm.reconfigure()
    print(\"Configuration reloaded\")
    return 0


def cmd_version(wm: WindowManager, args) -> int:
    \"\"\"Show version.\"\"\"
    print(f\"Openbox {wm.get_version()}\")
    return 0


def create_parser() -> argparse.ArgumentParser:
    \"\"\"Create argument parser.\"\"\"
    parser = argparse.ArgumentParser(
        prog=\"ob-python\",
        description=\"Openbox Python command-line interface\"
    )
    
    parser.add_argument(\"-v\", \"--verbose\", action=\"store_true\",
                       help=\"Verbose output\")
    parser.add_argument(\"--json\", action=\"store_true\",
                       help=\"Output as JSON\")
    parser.add_argument(\"--version\", action=\"store_true\",
                       help=\"Show version\")
    
    subparsers = parser.add_subparsers(dest=\"command\", help=\"Commands\")
    
    # list
    list_parser = subparsers.add_parser(\"list\", help=\"List windows\")
    list_parser.set_defaults(func=cmd_list)
    
    # focus
    focus_parser = subparsers.add_parser(\"focus\", help=\"Focus window\")
    focus_parser.add_argument(\"window\", help=\"Window ID\")
    focus_parser.set_defaults(func=cmd_focus)
    
    # close
    close_parser = subparsers.add_parser(\"close\", help=\"Close window\")
    close_parser.add_argument(\"window\", help=\"Window ID\")
    close_parser.set_defaults(func=cmd_close)
    
    # desktops
    desktops_parser = subparsers.add_parser(\"desktops\", help=\"List desktops\")
    desktops_parser.set_defaults(func=cmd_desktops)
    
    # switch
    switch_parser = subparsers.add_parser(\"switch\", help=\"Switch desktop\")
    switch_parser.add_argument(\"desktop\", help=\"Desktop number\")
    switch_parser.set_defaults(func=cmd_switch)
    
    # active
    active_parser = subparsers.add_parser(\"active\", help=\"Show active window\")
    active_parser.set_defaults(func=cmd_active)
    
    # execute
    exec_parser = subparsers.add_parser(\"execute\", help=\"Execute command\")
    exec_parser.add_argument(\"command\", nargs=\"+\", help=\"Command to execute\")
    exec_parser.set_defaults(func=cmd_execute)
    
    # reconfigure
    reconf_parser = subparsers.add_parser(\"reconfigure\", help=\"Reconfigure Openbox\")
    reconf_parser.set_defaults(func=cmd_reconfigure)
    
    return parser


def main() -> int:
    \"\"\"Main entry point.\"\"\"
    parser = create_parser()
    args = parser.parse_args()
    
    if args.version:
        print(\"ob-python 4.0.0\")
        return 0
    
    if not args.command:
        parser.print_help()
        return 1
    
    try:
        wm = WindowManager()
    except ConnectionError as e:
        print(f\"Error: {e}\", file=sys.stderr)
        return 2
    
    try:
        return args.func(wm, args)
    except OpenboxError as e:
        print(f\"Error: {e}\", file=sys.stderr)
        return 1


if __name__ == \"__main__\":
    sys.exit(main())
