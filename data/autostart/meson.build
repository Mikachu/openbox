# autostart - Meson Build

# Configure autostart script from template
autostart_in = configure_file(
  input : 'autostart.in',
  output : 'autostart',
  configuration : {
    'version': openbox_version,
    'configdir': configdir,
    'rcdir': rcdir,
    'libexecdir': libexecdir,
    'bindir': bindir,
  },
)

install_data(autostart_in,
  install_dir : rcdir,
  install_mode : 'rwxr-xr-x'
)

# Configure openbox-autostart script
openbox_autostart_in = configure_file(
  input : 'openbox-autostart.in',
  output : 'openbox-autostart',
  configuration : {
    'version': openbox_version,
    'configdir': configdir,
    'rcdir': rcdir,
    'libexecdir': libexecdir,
    'bindir': bindir,
  },
)

install_data(openbox_autostart_in,
  install_dir : libexecdir,
  install_mode : 'rwxr-xr-x'
)

# Static script
install_data('openbox-xdg-autostart',
  install_dir : libexecdir,
  install_mode : 'rwxr-xr-x'
)
