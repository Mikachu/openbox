# Openbox Utility Library (obt) - Meson Build

obt_sources = [
  'bsearch.h',
  'display.c',
  'display.h',
  'internal.h',
  'keyboard.c',
  'keyboard.h',
  'xml.c',
  'xml.h',
  'ddparse.c',
  'ddparse.h',
  'link.c',
  'link.h',
  'paths.c',
  'paths.h',
  'prop.c',
  'prop.h',
  'signal.c',
  'signal.h',
  'util.h',
  'xqueue.c',
  'xqueue.h',
]

obt_cflags = [
  '-DG_LOG_DOMAIN="Obt"',
]

obt_deps = [
  glib_dep,
  x11_dep,
  libxml_dep,
]

# Add X11 extension dependencies
if have_xkb
  obt_cflags += '-DXKB'
endif

if have_xinerama
  obt_deps += xinerama_dep
  obt_cflags += '-DXINERAMA'
endif

if have_xrandr
  obt_deps += xrandr_dep
  obt_cflags += '-DXRANDR'
endif

if have_xshape
  obt_cflags += '-DSHAPE'
endif

if have_xsync
  obt_cflags += '-DSYNC'
endif

# Build the obt library
obt_lib = library('obt',
  obt_sources,
  c_args : obt_cflags,
  dependencies : obt_deps,
  version : obt_version,
  soversion : obt_sover,
  install : true,
)

# Install headers
obt_headers = [
  'bsearch.h',
  'display.h',
  'keyboard.h',
  'xml.h',
  'link.h',
  'paths.h',
  'prop.h',
  'signal.h',
  'util.h',
  'version.h',
  'xqueue.h',
]

install_headers(obt_headers, subdir : 'openbox/@0@/obt'.format(obt_version))

# pkg-config file
pkg = import('pkgconfig')
pkg.generate(obt_lib,
  name : 'obt',
  description : 'Openbox Utility Library',
  version : obt_version,
  filebase : 'obt-@0@'.format(obt_version),
  subdirs : 'openbox/@0@'.format(obt_version),
)

# Unit tests
if get_option('tests')
  obt_unittests_sources = [
    'unittest_base.c',
    'unittest_base.h',
    'bsearch_unittest.c',
  ]

  executable('obt-unittests',
    obt_unittests_sources,
    c_args : obt_cflags + ['-DG_LOG_DOMAIN="Obt-Unittests"'],
    dependencies : [glib_dep, obt_lib],
    export_dynamic : true,
  )
endif
