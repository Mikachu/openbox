# Openbox Window Manager - Meson Build System
# Modern replacement for GNU Autotools (2026)

project('openbox', 'c',
  version : '3.7.0',
  license : 'GPL-2.0-or-later',
  meson_version : '>= 0.60.0',
  default_options : [
    'c_std=c99',
    'warning_level=2',
    'buildtype=release',
  ]
)

# Version information
openbox_version = meson.project_version()
version_parts = openbox_version.split('.')

# Library versioning (obt and obrender)
# Format: current.revision.age
obt_version = '3.6.5'
obt_sover = '3'
rr_version = '3.6.33'
rr_sover = '3'

# Gettext configuration
gettext_package = 'openbox'
localedir = get_option('localedir')
datadir = get_option('datadir')
configdir = get_option('sysconfdir') / 'xdg'
rcdir = configdir / 'openbox'
prefix = get_option('prefix')
bindir = get_option('bindir')
libexecdir = get_option('libexecdir')
includedir = get_option('includedir')
libdir = get_option('libdir')

# Compiler checks
cc = meson.get_compiler('c')
host_system = host_machine.system()

# Feature options
enable_startup_notification = get_option('startup-notification')
enable_xcursor = get_option('xcursor')
enable_imlib2 = get_option('imlib2')
enable_librsvg = get_option('librsvg')
enable_session_management = get_option('session-management')
enable_xkb = get_option('xkb')
enable_xrandr = get_option('xrandr')
enable_xshape = get_option('xshape')
enable_xinerama = get_option('xinerama')
enable_xsync = get_option('xsync')
enable_xauth = get_option('xauth')

# Debug and development options
enable_debug = get_option('debug')
enable_gprof = get_option('gprof')

# =============================================================================
# Required Dependencies
# =============================================================================

# X11 is always required
x11_dep = dependency('x11', required : true)

# GLib (required)
glib_dep = dependency('glib-2.0', version : '>= 2.28.0', required : true)
gthread_dep = dependency('gthread-2.0', required : true)

# Pango (required)
pango_dep = dependency('pango', version : '>= 1.8.0', required : true)
pangoxft_dep = dependency('pangoxft', version : '>= 1.8.0', required : true)

# libxml2 (required)
libxml_dep = dependency('libxml-2.0', version : '>= 2.6.0', required : true)

# =============================================================================
# Optional Dependencies
# =============================================================================

# Startup notification
libsn_dep = dependency('libstartup-notification-1.0', version : '>= 0.8', required : enable_startup_notification)
have_startup_notification = libsn_dep.found()

# X Cursor library
xcursor_dep = dependency('xcursor', required : enable_xcursor)
have_xcursor = xcursor_dep.found()

# Imlib2 for image loading
imlib2_dep = dependency('imlib2', required : enable_imlib2)
have_imlib2 = imlib2_dep.found()

# librsvg for SVG support
librsvg_dep = dependency('librsvg-2.0', required : enable_librsvg)
have_librsvg = librsvg_dep.found()

# Session management
sm_dep = dependency('sm', required : enable_session_management)
ice_dep = dependency('ice', required : enable_session_management)
have_session_management = sm_dep.found() and ice_dep.found()

# X11 extensions
xext_dep = dependency('xext', required : false)
xrender_dep = dependency('xrender', required : false)

# XKB extension
if enable_xkb
  xkb_dep = cc.find_library('X11', required : false)
  have_xkb = cc.has_header('X11/XKBlib.h', dependencies : x11_dep)
else
  have_xkb = false
endif

# XRandR extension
if enable_xrandr and xext_dep.found() and xrender_dep.found()
  xrandr_dep = dependency('xrandr', required : false)
  have_xrandr = xrandr_dep.found() and cc.has_header('X11/extensions/Xrandr.h', dependencies : x11_dep)
else
  have_xrandr = false
endif

# XShape extension
if enable_xshape and xext_dep.found()
  have_xshape = cc.has_header('X11/extensions/shape.h', dependencies : x11_dep)
else
  have_xshape = false
endif

# Xinerama extension
if enable_xinerama and xext_dep.found()
  xinerama_dep = dependency('xinerama', required : false)
  have_xinerama = xinerama_dep.found() and cc.has_header('X11/extensions/Xinerama.h', dependencies : x11_dep)
else
  have_xinerama = false
endif

# XSync extension
if enable_xsync and xext_dep.found()
  have_xsync = cc.has_header('X11/extensions/sync.h', dependencies : x11_dep)
else
  have_xsync = false
endif

# XAuth extension
xau_dep = dependency('xau', required : enable_xauth)
have_xauth = xau_dep.found()

# =============================================================================
# Configuration Header
# =============================================================================

config_h = configuration_data()

# Version info
config_h.set('PACKAGE_NAME', '"openbox"')
config_h.set('PACKAGE_VERSION', '"@0@"'.format(openbox_version))
config_h.set('PACKAGE_BUGREPORT', '"http://bugzilla.icculus.org"')
config_h.set('GETTEXT_PACKAGE', '"@0@"'.format(gettext_package))

# Paths
config_h.set('LOCALEDIR', '"@0@"'.format(localedir))
config_h.set('DATADIR', '"@0@"'.format(datadir))
config_h.set('CONFIGDIR', '"@0@"'.format(configdir))

# Optional features
config_h.set('USE_LIBSN', have_startup_notification ? 1 : false)
config_h.set('USE_XCURSOR', have_xcursor ? 1 : false)
config_h.set('USE_IMLIB2', have_imlib2 ? 1 : false)
config_h.set('USE_LIBRSVG', have_librsvg ? 1 : false)
config_h.set('USE_SM', have_session_management ? 1 : false)

# X11 extensions
config_h.set('XKB', have_xkb ? 1 : false)
config_h.set('XRANDR', have_xrandr ? 1 : false)
config_h.set('SHAPE', have_xshape ? 1 : false)
config_h.set('XINERAMA', have_xinerama ? 1 : false)
config_h.set('SYNC', have_xsync ? 1 : false)
config_h.set('AUTH', have_xauth ? 1 : false)

# Debug build
if enable_debug
  config_h.set('DEBUG', 1)
else
  config_h.set('NDEBUG', 1)
  config_h.set('G_DISABLE_ASSERT', 1)
endif

# Check for headers
config_h.set('HAVE_FCNTL_H', cc.has_header('fcntl.h'))
config_h.set('HAVE_SIGNAL_H', cc.has_header('signal.h'))
config_h.set('HAVE_STDLIB_H', cc.has_header('stdlib.h'))
config_h.set('HAVE_LOCALE_H', cc.has_header('locale.h'))
config_h.set('HAVE_SYS_STAT_H', cc.has_header('sys/stat.h'))
config_h.set('HAVE_SYS_WAIT_H', cc.has_header('sys/wait.h'))
config_h.set('HAVE_UNISTD_H', cc.has_header('unistd.h'))

configure_file(
  output : 'config.h',
  configuration : config_h,
)

# =============================================================================
# Compiler Flags
# =============================================================================

common_cflags = [
  '-DGETTEXT_PACKAGE="@0@"'.format(gettext_package),
  '-DLOCALEDIR="@0@"'.format(localedir),
  '-DDATADIR="@0@"'.format(datadir),
  '-DCONFIGDIR="@0@"'.format(configdir),
  '-DG_LOG_DOMAIN="Openbox"',
  '-fno-strict-aliasing',
]

if enable_debug
  common_cflags += [
    '-O0',
    '-ggdb',
    '-fno-inline',
    '-Wall',
    '-Wsign-compare',
    '-Waggregate-return',
    '-Wbad-function-cast',
    '-Wpointer-arith',
  ]
endif

if enable_gprof
  common_cflags += ['-pg', '-fno-inline']
endif

# Add common flags to global arguments
add_global_arguments(common_cflags, language : 'c')

# =============================================================================
# Subdirectories
# =============================================================================

subdir('obt')
subdir('obrender')
subdir('openbox')
subdir('tools')
subdir('data')
subdir('po')

# =============================================================================
# Summary
# =============================================================================

summary({
  'Openbox version': openbox_version,
  'Prefix': prefix,
  'Debug build': enable_debug,
}, section : 'Configuration')

summary({
  'Startup Notification': have_startup_notification,
  'X Cursor': have_xcursor,
  'Imlib2': have_imlib2,
  'SVG (librsvg)': have_librsvg,
  'Session Management': have_session_management,
}, section : 'Optional Features')

summary({
  'XKB': have_xkb,
  'XRandR': have_xrandr,
  'XShape': have_xshape,
  'Xinerama': have_xinerama,
  'XSync': have_xsync,
  'XAuth': have_xauth,
}, section : 'X11 Extensions')
