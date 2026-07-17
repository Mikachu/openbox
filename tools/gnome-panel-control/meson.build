# gnome-panel-control - Meson Build

gnome_panel_control_sources = ['gnome-panel-control.c']

gnome_panel_control_deps = [
  x11_dep,
]

executable('gnome-panel-control',
  gnome_panel_control_sources,
  dependencies : gnome_panel_control_deps,
  install : true,
  install_dir : bindir,
)
