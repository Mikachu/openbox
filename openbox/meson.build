# Openbox Window Manager - Meson Build

project('openbox', 'c',
  version: '4.0.0',
  license: 'GPL-2.0-or-later',
  meson_version: '>= 0.60.0',
  default_options: [
    'c_std=gnu11',
    'warning_level=2',
    'buildtype=release',
  ]
)

# Dependencies
glib_dep = dependency('glib-2.0', version: '>= 2.50')
gthread_dep = dependency('gthread-2.0')
gmodule_dep = dependency('gmodule-2.0')
json_glib_dep = dependency('json-glib-1.0', version: '>= 1.0', required: true)

# Optional dependencies for backends
x11_dep = dependency('x11', required: get_option('x11'))
xext_dep = dependency('xext', required: get_option('x11'))
xinerama_dep = dependency('xinerama', required: get_option('xinerama'))
xrandr_dep = dependency('xrandr', required: get_option('xrandr'))
xkbcommon_x11_dep = dependency('xkbcommon-x11', required: get_option('x11'))
wayland_client_dep = dependency('wayland-client', required: get_option('wayland'))
wayland_cursor_dep = dependency('wayland-cursor', required: get_option('wayland'))
wayland_protocols_dep = dependency('wayland-protocols', 
                                     required: get_option('wayland'))
libxml_dep = dependency('libxml-2.0', required: true)
cairo_dep = dependency('cairo', required: true)
pango_dep = dependency('pango', required: true)
pangoxft_dep = dependency('pangoxft', required: true)
libstartup_notification_dep = dependency('libstartup-notification-1.0', 
                                          required: get_option('startup_notification'))

# Configuration
conf_data = configuration_data()
conf_data.set('VERSION', meson.project_version())
conf_data.set('PACKAGE', 'openbox')
conf_data.set('SYSCONFDIR', get_option('prefix') / get_option('sysconfdir'))
conf_data.set('DATADIR', get_option('prefix') / get_option('datadir'))
conf_data.set('LOCALEDIR', get_option('prefix') / get_option('localedir'))

# Feature detection
if x11_dep.found()
  conf_data.set('HAVE_X11', 1)
endif
if wayland_client_dep.found()
  conf_data.set('HAVE_WAYLAND', 1)
endif
if json_glib_dep.found()
  conf_data.set('HAVE_JSON_GLIB', 1)
endif

configure_file(
  output: 'config.h',
  configuration: conf_data
)

# Include directories
inc = include_directories('.')

# Platform abstraction layer
platform_sources = []
platform_cflags = []

subdir('platform')

# Configuration sources
config_sources = [
  'config.c',
  'config_json.c',
  'config_format.c',
]

# Main openbox sources
openbox_sources = [
  'openbox.c',
  'client.c',
  'event.c',
  'frame.c',
  'grab.c',
  'keyboard.c',
  'menu.c',
  'misc.c',
  'mouse.c',
  'prop.c',
  'screen.c',
  'stacking.c',
  'startup.c',
  'translate.c',
  'window.c',
  'xevent.c',
] + platform_sources + config_sources

openbox_deps = [
  glib_dep,
  gthread_dep,
  gmodule_dep,
  json_glib_dep,
  libxml_dep,
  cairo_dep,
  pango_dep,
  pangoxft_dep,
]

if x11_dep.found()
  openbox_deps += [x11_dep, xext_dep]
  if xinerama_dep.found()
    openbox_deps += xinerama_dep
  endif
  if xrandr_dep.found()
    openbox_deps += xrandr_dep
  endif
  if xkbcommon_x11_dep.found()
    openbox_deps += xkbcommon_x11_dep
  endif
endif

if wayland_client_dep.found()
  openbox_deps += [wayland_client_dep, wayland_cursor_dep]
endif

if libstartup_notification_dep.found()
  openbox_deps += libstartup_notification_dep
endif

# Main executable
openbox_exe = executable('openbox',
  openbox_sources,
  include_directories: inc,
  dependencies: openbox_deps,
  c_args: platform_cflags,
  install: true,
)

# Tools
tools_sources = [
  'tools/xml2json.c',
]

tools_deps = [
  glib_dep,
  json_glib_dep,
  libxml_dep,
]

executable('openbox-xml2json',
  tools_sources,
  dependencies: tools_deps,
  install: true,
  install_dir: get_option('bindir')
)

# Data files
install_data('data/rc.xml',
  install_dir: get_option('sysconfdir') / 'openbox'
)
install_data('data/rc.json',
  install_dir: get_option('sysconfdir') / 'openbox'
)
install_data('data/menu.xml',
  install_dir: get_option('sysconfdir') / 'openbox'
)
install_data('data/menu.json',
  install_dir: get_option('sysconfdir') / 'openbox'
)

# Schemas
install_data('data/schemas/rc-schema.json',
  install_dir: get_option('datadir') / 'openbox' / 'schemas'
)
install_data('data/schemas/menu-schema.json',
  install_dir: get_option('datadir') / 'openbox' / 'schemas'
)

# Documentation
install_data('docs/CONFIGURATION_MIGRATION.md',
  install_dir: get_option('datadir') / 'doc' / 'openbox'
)
