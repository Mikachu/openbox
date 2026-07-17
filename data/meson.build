# Openbox Data Files - Meson Build

# Subdirectories
subdir('autostart')
subdir('gnome-session')
subdir('gnome-wm-properties')
subdir('xsession')

# XBM files for default buttons
xbm_files = [
  'xbm/bullet.xbm',
  'xbm/close.xbm',
  'xbm/desk.xbm',
  'xbm/desk_toggled.xbm',
  'xbm/iconify.xbm',
  'xbm/max.xbm',
  'xbm/max_toggled.xbm',
  'xbm/shade.xbm',
  'xbm/shade_toggled.xbm',
]

install_data(xbm_files,
  install_dir : datadir / 'doc' / 'openbox' / 'xbm'
)

# Default configuration files
install_data('rc.xml',
  install_dir : rcdir
)

install_data('menu.xml',
  install_dir : rcdir
)

# Schema files
install_data('rc.xsd',
  install_dir : datadir / 'openbox'
)

install_data('menu.xsd',
  install_dir : datadir / 'openbox'
)

# Desktop files
install_data('openbox.desktop',
  install_dir : datadir / 'applications'
)

# Pixmap
install_data('openbox.png',
  install_dir : datadir / 'pixmaps'
)

# Environment script
install_data('environment',
  install_dir : rcdir,
  install_mode : 'rwxr-xr-x'
)
