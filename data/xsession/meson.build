# xsession - Meson Build

# Configure session scripts and desktop files

session_scripts = [
  'openbox-session',
  'openbox-gnome-session',
  'openbox-kde-session',
]

foreach script : session_scripts
  configure_file(
    input : script + '.in',
    output : script,
    configuration : {
      'version': openbox_version,
      'configdir': configdir,
      'rcdir': rcdir,
      'libexecdir': libexecdir,
      'bindir': bindir,
    },
    install : true,
    install_dir : bindir,
    install_mode : 'rwxr-xr-x'
  )
endforeach

# Desktop files
desktop_files = [
  'openbox.desktop',
  'openbox-gnome.desktop',
  'openbox-kde.desktop',
]

foreach desktop : desktop_files
  configure_file(
    input : desktop + '.in',
    output : desktop,
    configuration : {
      'version': openbox_version,
      'configdir': configdir,
      'rcdir': rcdir,
      'libexecdir': libexecdir,
      'bindir': bindir,
    },
    install : true,
    install_dir : datadir / 'xsessions'
  )
endforeach
