# Platform Abstraction Layer - Meson Build

# Common sources
platform_sources = [
  'ob_platform.c',
  'ob_platform.h',
]

# X11 backend
if have_x11
  x11_sources = [
    'x11/x11_display.c',
    'x11/x11_events.c',
    'x11/x11_window.c',
    'x11/x11_backend.h',
  ]
  
  platform_sources += x11_sources
  
  platform_cflags += [
    '-DHAVE_X11_BACKEND',
  ]
endif

# Wayland backend
if have_wayland
  wayland_sources = [
    'wayland/wayland_display.c',
    'wayland/wayland_events.c',
    'wayland/wayland_window.c',
    'wayland/wayland_backend.h',
  ]
  
  # Generated protocol headers
  wayland_protocols = [
    'xdg-shell',
    'xdg-decoration',
  ]
  
  foreach proto : wayland_protocols
    # Generate headers from wayland-protocols
    wayland_header = custom_target(
      proto + '_client_header',
      input : wayland_protocols_dir / proto + '.xml',
      output : proto + '-client-protocol.h',
      command : [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
    )
    
    wayland_code = custom_target(
      proto + '_client_code',
      input : wayland_protocols_dir / proto + '.xml',
      output : proto + '-client-protocol.c',
      command : [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
    )
    
    wayland_sources += [wayland_header, wayland_code]
  endforeach
  
  platform_sources += wayland_sources
  
  platform_cflags += [
    '-DHAVE_WAYLAND_BACKEND',
  ]
endif

# Platform library (part of openbox)
openbox_sources += platform_sources
