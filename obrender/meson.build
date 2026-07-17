# Openbox Renderer - Meson Build

obrender_sources = [
  'color.c',
  'color.h',
  'font.c',
  'font.h',
  'gradient.c',
  'gradient.h',
  'icon.c',
  'icon.h',
  'mask.c',
  'mask.h',
  'render.c',
  'render.h',
  'theme.c',
  'theme.h',
  'texture.c',
  'texture.h',
  # CSS theme support
  'css_theme.c',
  'css_theme.h',
  'css_parser.c',
]

obrender_deps = [
  glib_dep,
  gobject_dep,
  cairo_dep,
  pango_dep,
  pangoxft_dep,
  libxml_dep,
  json_glib_dep,
  x11_dep,
  xext_dep,
]

# Build library
obrender_lib = static_library('obrender',
  obrender_sources,
  include_directories: inc,
  dependencies: obrender_deps,
)

# Export include for main openbox
obrender_inc = include_directories('.')
