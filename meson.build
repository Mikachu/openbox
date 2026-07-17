# Openbox Window Manager - Meson Build System v4.0.1
# Modern build with Rust core and C extensions
#
# IMPROVEMENTS v4.0.1:
# - Rust/Cargo integration for core components
# - Better feature detection and error messages
# - Improved cross-compilation support
# - Parallel builds with proper dependency ordering
# - Better Windows/macOS support (experimental)

project('openbox', 'c', 'rust',
  version : '4.0.1',
  license : 'GPL-2.0-or-later',
  meson_version : '>= 0.60.0',
  default_options : [
    'c_std=c11',
    'warning_level=2',
    'werror=false',
    'buildtype=release',
    'optimization=2',
  ]
)

# Version information
openbox_version = meson.project_version()
version_parts = openbox_version.split('.')
openbox_major = version_parts[0]
openbox_minor = version_parts[1]
openbox_patch = version_parts[2]

# Library versioning for C extensions
# Format: current.revision.age
ob_version = '4.0.1'
ob_sover = '4'

# Installation directories
prefix = get_option('prefix')
bindir = get_option('bindir')
libdir = get_option('libdir')
includedir = get_option('includedir')
datadir = get_option('datadir')
localedir = get_option('localedir')
sysconfdir = get_option('sysconfdir')
configdir = sysconfdir / 'xdg' / 'openbox'
libexecdir = get_option('libexecdir')

# Feature options with improved descriptions
option_portal = get_option('portal')
option_extensions = get_option('extensions')
option_python = get_option('python-bindings')
option_docs = get_option('documentation')
option_tests = get_option('tests')
option_coverage = get_option('coverage')

# =============================================================================
# Compilers and Build Tools
# =============================================================================

cc = meson.get_compiler('c')
rust = meson.get_compiler('rust')
host_system = host_machine.system()
cpu_family = host_machine.cpu_family()

# Check for required tools
cargo = find_program('cargo', version : '>= 1.70', required : true)
rustc = find_program('rustc', version : '>= 1.70', required : true)
python3 = find_program('python3', version : '>= 3.8', required : true)

# =============================================================================
# Feature Detection - IMPROVED v4.0.1
# =============================================================================

# Helper function for better feature detection
def detect_feature(feature_name, dep_name, version_req, required, description):
    dep = dependency(dep_name, version : version_req, required : required)
    if dep.found():
        message(f'Feature {feature_name}: ENABLED')
        message(f'  {description}')
    else:
        if required
            error(f'Required dependency {dep_name} not found')
        else
            message(f'Feature {feature_name}: DISABLED')
    return dep

# Core dependencies with version checking
glib_dep = detect_feature('GLib core', 'glib-2.0', '>= 2.50', true, 
    'Core object system and main loop')
gthread_dep = detect_feature('GLib threading', 'gthread-2.0', '>= 2.50', true,
    'Thread support for GLib')
gobject_dep = detect_feature('GObject', 'gobject-2.0', '>= 2.50', true,
    'Object system for D-Bus integration')

# X11 dependencies
x11_dep = detect_feature('X11 core', 'x11', '>= 1.6', true,
    'X11 protocol support')
xinerama_dep = detect_feature('Xinerama', 'xinerama', '>= 1.1', false,
    'Multi-monitor support')  # Optional in 4.0.1
xrandr_dep = detect_feature('XRandR', 'xrandr', '>= 1.5', true,
    'Display configuration and hotplug')
xkb_dep = detect_feature('XKB', 'xkbcommon', '>= 0.7', true,
    'Keyboard handling')
xkb_x11_dep = detect_feature('XKB X11', 'xkbcommon-x11', '>= 0.7', true,
    'XKB X11 integration')

# Graphics and text
cairo_dep = detect_feature('Cairo', 'cairo', '>= 1.14', true,
    '2D graphics rendering')
pango_dep = detect_feature('Pango', 'pango', '>= 1.40', true,
    'Text layout and rendering')
pangocairo_dep = detect_feature('PangoCairo', 'pangocairo', '>= 1.40', true,
    'Pango Cairo integration')

# D-Bus for IPC
dbus_dep = detect_feature('D-Bus', 'dbus-1', '>= 1.10', true,
    'Inter-process communication')

# Portal support
portal_deps = []
if option_portal
    portal_deps += detect_feature('Portal GLib', 'gio-2.0', '>= 2.50', true,
        'GIO for portal implementation')
    portal_deps += detect_feature('Portal Unix', 'gio-unix-2.0', '>= 2.50', true,
        'Unix-specific GIO functions')
endif

# =============================================================================
# Configuration Data
# =============================================================================

conf_data = configuration_data()
conf_data.set('VERSION', openbox_version)
conf_data.set('VERSION_MAJOR', openbox_major)
conf_data.set('VERSION_MINOR', openbox_minor)
conf_data.set('VERSION_PATCH', openbox_patch)
conf_data.set('PACKAGE', 'openbox')
conf_data.set('PACKAGE_BUGREPORT', 'https://github.com/openbox/openbox/issues')
conf_data.set('GETTEXT_PACKAGE', 'openbox')
conf_data.set('LOCALEDIR', localedir)
conf_data.set('SYSCONFDIR', sysconfdir)
conf_data.set('DATADIR', datadir)
conf_data.set('LIBDIR', libdir)
conf_data.set('BINDIR', bindir)
conf_data.set('CONFIGDIR', configdir)

# Feature flags
conf_data.set('HAVE_XINERAMA', xinerama_dep.found())
conf_data.set('HAVE_PORTAL', option_portal)
conf_data.set('HAVE_EXTENSIONS', option_extensions)
conf_data.set('HAVE_PYTHON', option_python)

# =============================================================================
# Rust Build - IMPROVED v4.0.1
# =============================================================================

# Cargo configuration for cross-compilation
cargo_target = ''
if host_machine.system() == 'linux'
    if host_machine.cpu_family() == 'x86_64'
        cargo_target = 'x86_64-unknown-linux-gnu'
    elif host_machine.cpu_family() == 'x86'
        cargo_target = 'i686-unknown-linux-gnu'
    elif host_machine.cpu_family() == 'aarch64'
        cargo_target = 'aarch64-unknown-linux-gnu'
    elif host_machine.cpu_family() == 'arm'
        cargo_target = 'armv7-unknown-linux-gnueabihf'
    endif
elif host_machine.system() == 'darwin'
    cargo_target = 'x86_64-apple-darwin'
elif host_machine.system() == 'windows'
    cargo_target = 'x86_64-pc-windows-gnu'
endif

# Rust build target directory
rust_target_dir = meson.current_build_dir() / 'rust-target'
cargo_manifest = meson.current_source_dir() / 'Cargo.toml'

# Custom target for Rust build with proper dependencies
cargo_build = custom_target('openbox-core',
    output : ['libopenbox_core.a', 'openbox'],
    command : [
        cargo, 'build',
        '--manifest-path', cargo_manifest,
        '--target-dir', rust_target_dir,
        cargo_target != '' ? '--target' : '', cargo_target,
        '--release',
    ],
    env : {
        'CARGO_TARGET_DIR': rust_target_dir,
        'RUSTFLAGS': '-C link-arg=-s',  # Strip symbols in release
    },
    install : false,
    build_by_default : true,
)

# Rust library for linking
openbox_core_lib = rust_target_dir / 'release' / 'libopenbox_core.a'

if cargo_target != ''
    openbox_core_lib = rust_target_dir / cargo_target / 'release' / 'libopenbox_core.a'
endif

# =============================================================================
# C Extension Library
# =============================================================================

# Extension API headers
install_headers(
    'include/openbox/extension.h',
    'include/openbox/client.h',
    'include/openbox/screen.h',
    subdir : 'openbox',
)

# Extension library (C ABI for extensions)
extension_sources = [
    'src/extension_api.c',
    'src/client.c',
    'src/screen.c',
]

extension_lib = shared_library('openbox',
    extension_sources,
    dependencies : [glib_dep, gobject_dep, cairo_dep],
    version : ob_version,
    soversion : ob_sover,
    install : true,
)

# pkg-config file
pkg = import('pkgconfig')
pkg.generate(extension_lib,
    name : 'openbox',
    description : 'Openbox window manager extension API',
    version : openbox_version,
    filebase : 'openbox',
    subdirs : 'openbox',
)

# =============================================================================
# Executables
# =============================================================================

# Main openbox binary (links Rust core + C extensions)
openbox_sources = ['src/main.c']

openbox_exe = executable('openbox',
    openbox_sources,
    link_with : extension_lib,
    dependencies : [
        glib_dep, gthread_dep, gobject_dep,
        x11_dep, xrandr_dep, xkb_dep, xkb_x11_dep,
        cairo_dep, pango_dep, pangocairo_dep,
        dbus_dep,
    ],
    link_args : [openbox_core_lib.full_path()],
    install : true,
    install_dir : bindir,
)

# obctl control utility
obctl_sources = ['src/obctl.c']

obctl_exe = executable('obctl',
    obctl_sources,
    dependencies : [dbus_dep, glib_dep],
    install : true,
)

# Portal service (if enabled)
if option_portal
    portal_sources = [
        'portal/openbox-portal.c',
    ]
    
    portal_exe = executable('openbox-portal',
        portal_sources,
        dependencies : portal_deps,
        install : true,
        install_dir : libexecdir,
    )
endif

# =============================================================================
# Python Bindings
# =============================================================================

if option_python
    py = import('python')
    python = py.find_installation('python3', required : true)
    
    python.extension_module('_openbox',
        'python/openbox.c',
        dependencies : [python.dependency(), dbus_dep, glib_dep],
        install : true,
        subdir : 'openbox',
    )
    
    python.install_sources(
        'python/openbox/__init__.py',
        'python/openbox/async_client.py',
        'python/openbox/sync_client.py',
        subdir : 'openbox',
    )
endif

# =============================================================================
# Tests
# =============================================================================

if option_tests
    test_deps = [glib_dep]
    
    # Unit tests
    test('extension_api',
        executable('test_extension_api',
            'tests/test_extension_api.c',
            link_with : extension_lib,
            dependencies : test_deps,
        ),
        timeout : 60,
    )
    
    # Rust tests via cargo
    test('rust_core',
        cargo,
        args : ['test', '--manifest-path', cargo_manifest],
        timeout : 300,
        is_parallel : false,  # Cargo handles parallelism
    )
    
    # Integration tests
    if option_python
        test('python_bindings',
            python,
            args : ['-m', 'pytest', 'tests/python/'],
            timeout : 120,
        )
    endif
endif

# =============================================================================
# Documentation
# =============================================================================

if option_docs
    subdir('doc')
endif

# =============================================================================
# Installation
# =============================================================================

# Desktop files
install_data(
    'data/xsessions/openbox.desktop',
    install_dir : datadir / 'xsessions',
)

# D-Bus service files
configure_file(
    input : 'data/dbus/org.openbox.WindowManager.service.in',
    output : 'org.openbox.WindowManager.service',
    configuration : conf_data,
    install_dir : datadir / 'dbus-1' / 'services',
)

# systemd user service
install_data(
    'systemd/openbox.service',
    install_dir : get_option('systemduserunitdir'),
)

# Themes
install_subdir('themes',
    install_dir : datadir / 'openbox',
)

# Config examples
install_data(
    'data/rc.xml',
    'data/menu.xml',
    'data/environment',
    install_dir : configdir,
)

# =============================================================================
# Summary
# =============================================================================

summary({
    'Version': openbox_version,
    'Prefix': prefix,
    'Binary dir': bindir,
    'Library dir': libdir,
    'Data dir': datadir,
    'Config dir': configdir,
}, section: 'Installation')

summary({
    'Portal support': option_portal,
    'Extensions': option_extensions,
    'Python bindings': option_python,
    'Documentation': option_docs,
    'Tests': option_tests,
    'Coverage': option_coverage,
}, section: 'Features')

summary({
    'Host system': host_system,
    'CPU family': cpu_family,
    'Rust target': cargo_target != '' ? cargo_target : 'native',
    'C compiler': cc.get_id() + ' ' + cc.version(),
    'Rust compiler': rustc.full_path() + ' ' + run_command(rustc, '--version', check: false).stdout().strip(),
}, section: 'Build')
