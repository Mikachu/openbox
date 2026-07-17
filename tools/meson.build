# Openbox Tools - Meson Build

subdir('gdm-control')
subdir('gnome-panel-control')
subdir('obxprop')

# themeupdate is a Python script, install it directly
install_data('themeupdate/themeupdate.py',
  install_dir : bindir,
  rename : 'themeupdate',
  install_mode : 'rwxr-xr-x',
)
