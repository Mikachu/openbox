# obxprop - Meson Build

obxprop_sources = ['obxprop.c']

obxprop_deps = [
  glib_dep,
  x11_dep,
]

executable('obxprop',
  obxprop_sources,
  dependencies : obxprop_deps,
  install : true,
  install_dir : bindir,
)
