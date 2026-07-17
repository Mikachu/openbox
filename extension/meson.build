# Extension system build configuration

extension_includes = include_directories('.')

# Extension API header (installed)
install_headers('openbox-extension.h',
n    subdir: 'openbox')

n# Extension loader (linked into Openbox core)
nextension_sources = [
n    'openbox-extension-loader.c',
n    'openbox-extension-events.c',  # Event subscription system
n]

n# Core API implementations (stubs that call into Openbox core)
nextension_core_api = [
n    'api/window-api.c',
n    'api/desktop-api.c',
n    'api/screen-api.c',
n    'api/config-api.c',
n    'api/theme-api.c',
n    'api/action-api.c',
n]

n# Build extension support into Openbox
nextension_lib = static_library('obextension',
n    extension_sources + extension_core_api,
n    include_directories: extension_includes,
n    dependencies: [glib, gthread],
n    install: false)

n# Example extensions (optional)
nif get_option('build-examples')
n    # Tiling extension
n    shared_library('tiling',
n        'examples/tiling.c',
n        include_directories: extension_includes,
n        install: true,
n        install_dir: extensiondir)
n    \n    # Window rules extension
n    shared_library('window-rules',
n        'examples/window-rules.c',
n        include_directories: extension_includes,
n        install: true,
n        install_dir: extensiondir)
n    \n    # Compositor effects extension
n    shared_library('compositor',
n        'examples/compositor.c',
n        include_directories: extension_includes,
n        dependencies: [cairo, pangocairo],
n        install: true,
n        install_dir: extensiondir)
nendif

n# Python bindings
nsubdir('python')
