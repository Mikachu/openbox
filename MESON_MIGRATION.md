# Openbox Meson Build System Migration

## Overview

This document describes the migration from GNU Autotools to Meson build system for Openbox 3.7+. This migration modernizes the build process while maintaining backward compatibility.

## Why Meson?

- **Faster builds**: Meson uses Ninja backend which is significantly faster than Make
- **Better cross-compilation**: Native support for cross-compilation with cross files
- **Modern syntax**: Readable, declarative syntax vs. M4 macros
- **Better dependency management**: First-class pkg-config and dependency support
- **Integrated testing**: Built-in test framework with `meson test`
- **IDE support**: Better integration with modern IDEs

## File Structure

```
openbox/
├── meson.build                 # Root build file
├── meson_options.txt           # Build options
├── configure                   # Autotools compatibility wrapper (generated)
├── Makefile                    # Autotools compatibility wrapper (generated)
├── scripts/
│   └── generate-autotools.sh   # Generate compatibility scripts
├── obt/
│   └── meson.build
├── obrender/
│   └── meson.build
├── openbox/
│   └── meson.build
├── tools/
│   ├── meson.build
│   ├── gdm-control/
│   │   └── meson.build
│   ├── gnome-panel-control/
│   │   └── meson.build
│   └── obxprop/
│       └── meson.build
├── data/
│   ├── meson.build
│   ├── autostart/
│   │   └── meson.build
│   ├── gnome-session/
│   │   └── meson.build
│   ├── gnome-wm-properties/
│   │   └── meson.build
│   └── xsession/
│       └── meson.build
└── po/
    └── meson.build
```

## Quick Start

### Standard Meson Workflow

```bash
# Configure the build
meson setup builddir --prefix=/usr/local

# Build
meson compile -C builddir
# or: ninja -C builddir

# Run tests
meson test -C builddir

# Install
meson install -C builddir
```

### Backward Compatibility (Autotools-style)

```bash
# Traditional configure && make workflow
./configure --prefix=/usr/local
make
make install
```

## Build Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `startup-notification` | feature | auto | Enable startup notification |
| `xcursor` | feature | auto | Enable X Cursor library |
| `imlib2` | feature | auto | Enable Imlib2 image loading |
| `librsvg` | feature | auto | Enable SVG support |
| `session-management` | feature | auto | Enable session management |
| `xkb` | feature | auto | Enable XKB extension |
| `xrandr` | feature | auto | Enable XRandR extension |
| `xshape` | feature | auto | Enable X Shape extension |
| `xinerama` | feature | auto | Enable Xinerama extension |
| `xsync` | feature | auto | Enable X Sync extension |
| `xauth` | feature | auto | Enable X Auth extension |
| `debug` | boolean | false | Enable debug build |
| `gprof` | boolean | false | Enable profiling |

### Example: Custom Build

```bash
meson setup builddir \
  --prefix=/usr \
  -Dimlib2=disabled \
  -Dlibrsvg=enabled \
  -Ddebug=true \
  -Dgprof=true
```

## Cross-Compilation

Create a cross file (e.g., `cross-arm64.txt`):

```ini
[binaries]
c = 'aarch64-linux-gnu-gcc'
cpp = 'aarch64-linux-gnu-g++'
ar = 'aarch64-linux-gnu-ar'
strip = 'aarch64-linux-gnu-strip'
pkgconfig = 'pkg-config'

[host_machine]
system = 'linux'
cpu_family = 'aarch64'
cpu = 'aarch64'
endian = 'little'

[properties]
c_args = ['-march=armv8-a']
```

Build with:

```bash
meson setup builddir --cross-file cross-arm64.txt
meson compile -C builddir
```

## Feature Parity with Autotools

### Translated Checks

| Autotools | Meson |
|-----------|-------|
| `AC_CHECK_HEADERS` | `cc.has_header()` |
| `AC_CHECK_LIB` | `cc.find_library()` |
| `PKG_CHECK_MODULES` | `dependency()` |
| `AC_ARG_ENABLE` | `get_option()` with `feature` type |
| `AC_DEFINE` | `config_h.set()` |
| `AM_CONDITIONAL` | `feature.found()` checks |
| `AC_CONFIG_FILES` | `configure_file()` |

### Library Versioning

Both `obt` and `obrender` maintain their original version schemes:

- **obt**: 3.6.5 (soversion 3)
- **obrender**: 3.6.33 (soversion 3)

### pkg-config Files

Generated automatically for:
- `obt-3.6.pc`
- `obrender-3.6.pc`

## Testing the Build

### 1. Basic Build Test

```bash
# Clean build
rm -rf builddir
meson setup builddir
meson compile -C builddir
```

### 2. Feature Detection Test

```bash
# Check configuration summary
meson configure builddir
```

### 3. Installation Test

```bash
# Install to temporary directory
DESTDIR=/tmp/openbox-test meson install -C builddir
find /tmp/openbox-test -type f | head -20
```

### 4. Backward Compatibility Test

```bash
# Test Autotools-style workflow
./configure --prefix=/usr/local
make
make check
make install DESTDIR=/tmp/openbox-test
```

## Migration Checklist

- [x] Root meson.build with all dependencies
- [x] meson_options.txt for optional features
- [x] obt/ subdirectory build
- [x] obrender/ subdirectory build
- [x] openbox/ subdirectory build
- [x] tools/ subdirectory build
- [x] data/ subdirectory build with template processing
- [x] po/ subdirectory build (i18n)
- [x] Autotools compatibility scripts (configure, Makefile)
- [x] Version header generation
- [x] pkg-config file generation
- [ ] CI/CD integration (GitHub Actions)
- [ ] Documentation updates

## Known Issues

1. **Python 3.6+ required**: Meson requires Python 3.6 or newer
2. **Ninja required**: Ninja 1.8+ is required for the build backend
3. **gettext**: Some systems may need `gettext` installed for i18n support

## Troubleshooting

### "meson: command not found"

Install Meson:
```bash
pip3 install meson
# or
apt-get install meson  # Debian/Ubuntu
dnf install meson       # Fedora
```

### Missing dependencies

Meson will report which dependencies are missing. Install them:
```bash
# Debian/Ubuntu
apt-get install libglib2.0-dev libpango1.0-dev libxml2-dev \
  libstartup-notification0-dev libxcursor-dev libimlib2-dev \
  librsvg2-dev libsm-dev libice-dev libxinerama-dev libxrandr-dev

# Fedora
dnf install glib2-devel pango-devel libxml2-devel \
  startup-notification-devel libXcursor-devel imlib2-devel \
  librsvg2-devel libSM-devel libICE-devel libXinerama-devel libXrandr-devel
```

### Cross-compilation issues

Ensure all cross-compiled dependencies have their `.pc` files available or specify them in the cross file.

## Future Enhancements

1. **Rust support**: Meson has excellent Rust integration for Project 5
2. **Wayland protocols**: Native code generation for Wayland protocols
3. **Static analysis**: Integration with clang-tidy, cppcheck
4. **Fuzzing**: Built-in fuzzing target support
5. **Sanitizers**: Easy ASan, UBSan, MSan integration

## References

- [Meson Documentation](https://mesonbuild.com/)
- [Meson Porting Guide](https://mesonbuild.com/Porting-from-autotools.html)
- [Openbox Original Build System](HACKING)

## Maintainer Notes

To regenerate Autotools compatibility scripts:
```bash
./scripts/generate-autotools.sh
```

To update version numbers, edit:
- `meson.build`: `version` in project()
- `obt/meson.build`: `obt_version`
- `obrender/meson.build`: `rr_version`
