Name:           openbox
Version:        4.0.0
Release:        1%{?dist}
Summary:        Highly configurable, next-generation stacking window manager

License:        GPL-2.0-or-later
URL:            https://openbox.org
Source0:        %{url}/releases/%{name}-%{version}.tar.gz

BuildRequires:  meson >= 0.60
BuildRequires:  ninja-build
BuildRequires:  gcc
BuildRequires:  rust-all >= 1.70
BuildRequires:  cargo
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gtk+-3.0)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(cairo)
BuildRequires:  pkgconfig(pango)
BuildRequires:  pkgconfig(pangocairo)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xinerama)
BuildRequires:  pkgconfig(xrandr)
BuildRequires:  pkgconfig(xcursor)
BuildRequires:  pkgconfig(xft)
BuildRequires:  pkgconfig(startup-notification-1.0)
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  pkgconfig(systemd)
BuildRequires:  python3-devel
BuildRequires:  python3-setuptools
BuildRequires:  python3-dbus

Requires:       python3-%{name} = %{version}-%{release}
Requires:       %{name}-themes = %{version}-%{release}

%description
Openbox is a window manager for the X11 windowing system. It is designed to be
small, fast, and fully compliant with the Inter-Client Communication
Conventions Manual (ICCCM) and Extended Window Manager Hints (EWMH).

Openbox 4 is a complete rewrite in Rust, featuring memory safety, CSS-based
theming, D-Bus IPC, extension API, and Python bindings.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    devel
This package contains the header files and static library for developing
Openbox extensions.

%package        -n python3-%{name}
Summary:        Python bindings for %{name}
Requires:       %{name} = %{version}-%{release}
Requires:       python3-dbus
Requires:       python3-gobject

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
Requires:       xdg-desktop-portal

%description    portal
Portal implementations for sandboxed applications under Openbox.

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

%check
%meson_test

%files
%license LICENSE
%doc README.md NEWS AUTHORS
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

%changelog
* Mon Jan 15 2024 Openbox Team <dev@openbox.org> - 4.0.0-1
- Initial release of Openbox 4
