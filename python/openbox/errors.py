\"\"\"Openbox Python exceptions.\"\"\"


class OpenboxError(Exception):
n    \"\"\"Base exception for Openbox errors.\"\"\"\n    pass


class ConnectionError(OpenboxError):
n    \"\"\"Failed to connect to Openbox.\"\"\"\n    pass


class WindowNotFoundError(OpenboxError):
n    \"\"\"Requested window not found.\"\"\"\n    pass


class DesktopNotFoundError(OpenboxError):
n    \"\"\"Requested desktop not found.\"\"\"\n    pass


class PermissionError(OpenboxError):
n    \"\"\"Operation not permitted.\"\"\"\n    pass


class ConfigError(OpenboxError):
n    \"\"\"Configuration error.\"\"\"\n    pass


class ExtensionError(OpenboxError):
n    \"\"\"Extension-related error.\"\"\"\n    pass


class ScriptError(OpenboxError):
n    \"\"\"Script execution error.\"\"\"\n    pass
