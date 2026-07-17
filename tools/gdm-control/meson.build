# gdm-control - Meson Build

gdm_control_sources = ['gdm-control.c']

gdm_control_deps = [
  xau_dep,
  x11_dep,
  glib_dep,
]

executable('gdm-control',
  gdm_control_sources,
  dependencies : gdm_control_deps,
  install : true,
  install_dir : bindir,
)
