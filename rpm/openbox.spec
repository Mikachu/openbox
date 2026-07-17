Name:           openbox
Version:        4.0.1
Release:        1%{?dist}
Summary:        Highly configurable, next-generation stacking window manager

License:        GPL-2.0-or-later
URL:            https://openbox.org
Source0:        %{url}/releases/%{name}-%{version}.tar.gz

BuildRequires:  meson >= 0.60
BuildRequires:  ninja-build
BuildRequires:  gcc
BuildRequires:  rust >= 1.70
BuildRequires:  cargo >= 1.70
BuildRequires:  pkgconfig(glib-2.0) >= 2.50
BuildRequires:  pkgconfig(gtk+-3.0)
BuildRequires:  pkgconfig(libxml-2.0) >= 2.9
BuildRequires:  pkgconfig(cairo) >= 1.14
BuildRequires:  pkgconfig(pango) >= 1.40
BuildRequires:  pkgconfig(pangocairo) >= 1.40
BuildRequires:  pkgconfig(x11) >= 1.6
BuildRequires:  pkgconfig(xinerama) >= 1.1
BuildRequires:  pkgconfig(xrandr) >= 1.5
BuildRequires:  pkgconfig(xcursor) >= 1.1
BuildRequires:  pkgconfig(xft) >= 2.3
BuildRequires:  pkgconfig(startup-notification-1.0) >= 0.12
BuildRequires:  pkgconfig(dbus-1) >= 1.10
BuildRequires:  pkgconfig(systemd) >= 232
BuildRequires:  pkgconfig(xkbcommon) >= 0.7
BuildRequires:  pkgconfig(xkbcommon-x11) >= 0.7
BuildRequires:  python3-devel >= 3.8
BuildRequires:  python3-setuptools
BuildRequires:  python3-dbus
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib

Requires:       python3-%{name} = %{version}-%{release}
Requires:       %{name}-themes = %{version}-%{release}
Requires:       dbus-daemon
Requires:       xorg-x11-server-utils
Requires:       xorg-x11-utils

Recommends:     xdg-desktop-portal-gtk
Recommends:     dunst
Recommends:     policycoreutils-python-utils

%description
Openbox is a window manager for the X11 windowing system. It is designed to be
small, fast, and fully compliant with the Inter-Client Communication
Conventions Manual (ICCCM) and Extended Window Manager Hints (EWMH).

Openbox 4 is a complete rewrite in Rust, featuring memory safety, CSS-based
theming, D-Bus IPC, extension API, and Python bindings.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       glib2-devel
Requires:       cairo-devel

%description    devel
This package contains the header files and static library for developing
Openbox extensions and plugins.

%package        -n python3-%{name}
Summary:        Python bindings for %{name}
Requires:       %{name} = %{version}-%{release}
Requires:       python3-dbus
Requires:       python3-gobject
Requires:       python3-typing-extensions

%description    -n python3-%{name}
Python bindings for controlling Openbox via D-Bus.

%package        themes
Summary:        Themes for %{name}
BuildArch:      noarch

%description    themes
Default themes for Openbox.

%package        portal
Summary:        xdg-desktop-portal backend for %{name}
Requires:       %{name} = %{version}-%{release}
Requires:       xdg-desktop-portal >= 1.10

%description    portal
Portal implementations for sandboxed applications under Openbox.

%package        doc
Summary:        Documentation for %{name}
BuildArch:      noarch

%description    doc
Documentation for Openbox.

%prep
%autosetup

%build
%meson \
    -Dportal=true \
    -Dextensions=true \
    -Dpython-bindings=true
%meson_build

%install
%meson_install

# Install Python bindings
cd python
%py3_install
cd ..

# Validate desktop file
desktop-file-validate %{buildroot}%{_datadir}/applications/*.desktop

# Validate AppStream metadata
appstream-util validate-relax --nonet %{buildroot}%{_datadir}/metainfo/*.xml

%check
%meson_test

%files
%license LICENSE
%doc README.md NEWS AUTHORS CHANGELOG.md
%{_bindir}/openbox
%{_bindir}/obctl
%{_bindir}/openbox-session
%{_libdir}/libopenbox.so.*
%{_libdir}/openbox/extensions/
%{_datadir}/openbox/
%{_datadir}/xsessions/openbox.desktop
%{_datadir}/wayland-sessions/openbox.desktop
%{_mandir}/man1/openbox.1*
%{_mandir}/man1/obctl.1*
%{_mandir}/man5/rc.xml.5*
%{_mandir}/man5/menu.xml.5*
%{_datadir}/dbus-1/services/org.openbox.WindowManager.service
%{_userunitdir}/openbox.service
%{_datadir}/metainfo/org.openbox.Openbox.metainfo.xml

%files devel
%{_includedir}/openbox/
%{_libdir}/libopenbox.so
%{_libdir}/pkgconfig/openbox.pc

%files -n python3-%{name}
%{python3_sitelib}/openbox/
%{python3_sitelib}/openbox-*.egg-info/

%files themes
%{_datadir}/themes/Openbox*/

%files portal
%{_libdir}/xdg-desktop-portal/openbox-portal
%{_datadir}/dbus-1/services/org.freedesktop.impl.portal.desktop.openbox.service
%{_datadir}/xdg-desktop-portal/portals/openbox.portal

%files doc
%{_docdir}/openbox/

%changelog
* Mon Jan 22 2024 Openbox Team <dev@openbox.org> - 4.0.1-1
- Bug fix release with stability improvements
- Fixed memory leaks in window management
- Improved D-Bus connection stability
- Fixed CSS parser crash on malformed files
- Performance optimizations

* Mon Jan 15 2024 Openbox Team <dev@openbox.org> - 4.0.0-1
- Initial release of Openbox 4
