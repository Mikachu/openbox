#!/bin/bash
# generate-autotools.sh - Generate Autotools-compatible configure script from Meson
# This provides backward compatibility during the transition period

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "Generating Autotools-compatible wrapper for Openbox..."
echo "This script wraps Meson to provide configure-like interface"

cat > "$PROJECT_ROOT/configure" << 'CONFIGURE_EOF'
#!/bin/bash
# configure - Autotools-compatible wrapper for Meson
# Generated automatically from Meson build files

set -e

# Default values
PREFIX=/usr/local
BINDIR=
DATADIR=
SYSCONFDIR=
LIBDIR=
INCLUDEDIR=
ENABLE_DEBUG=no
ENABLE_NLS=yes

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --prefix=*)
            PREFIX="${1#*=}"
            shift
            ;;
        --bindir=*)
            BINDIR="${1#*=}"
            shift
            ;;
        --datadir=*)
            DATADIR="${1#*=}"
            shift
            ;;
        --sysconfdir=*)
            SYSCONFDIR="${1#*=}"
            shift
            ;;
        --libdir=*)
            LIBDIR="${1#*=}"
            shift
            ;;
        --includedir=*)
            INCLUDEDIR="${1#*=}"
            shift
            ;;
        --enable-debug)
            ENABLE_DEBUG=yes
            shift
            ;;
        --disable-debug)
            ENABLE_DEBUG=no
            shift
            ;;
        --enable-nls)
            ENABLE_NLS=yes
            shift
            ;;
        --disable-nls)
            ENABLE_NLS=no
            shift
            ;;
        --help)
            echo "Usage: ./configure [options]"
            echo ""
            echo "Options:"
            echo "  --prefix=PREFIX         Install architecture-independent files in PREFIX"
            echo "  --bindir=DIR            User executables [PREFIX/bin]"
            echo "  --datadir=DIR           Read-only architecture-independent data [PREFIX/share]"
            echo "  --sysconfdir=DIR        Read-only single-machine data [PREFIX/etc]"
            echo "  --libdir=DIR            Object code libraries [PREFIX/lib]"
            echo "  --includedir=DIR        C header files [PREFIX/include]"
            echo "  --enable-debug          Enable debugging features"
            echo "  --disable-nls           Disable Native Language Support"
            echo "  --help                  Display this help and exit"
            echo ""
            echo "This is a Meson-based build system. Options are translated to meson setup."
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Try './configure --help' for more information."
            exit 1
            ;;
    esac
done

# Build meson arguments
MESON_ARGS=()

# Prefix
MESON_ARGS+=("--prefix=$PREFIX")

# Directories
[[ -n "$BINDIR" ]] && MESON_ARGS+=("--bindir=$BINDIR")
[[ -n "$DATADIR" ]] && MESON_ARGS+=("--datadir=$DATADIR")
[[ -n "$SYSCONFDIR" ]] && MESON_ARGS+=("--sysconfdir=$SYSCONFDIR")
[[ -n "$LIBDIR" ]] && MESON_ARGS+=("--libdir=$LIBDIR")
[[ -n "$INCLUDEDIR" ]] && MESON_ARGS+=("--includedir=$INCLUDEDIR")

# Build type
if [[ "$ENABLE_DEBUG" == "yes" ]]; then
    MESON_ARGS+=("--buildtype=debug")
else
    MESON_ARGS+=("--buildtype=release")
fi

# Create build directory
BUILD_DIR="builddir"

echo "Configuring Openbox with Meson..."
echo "  prefix: $PREFIX"
echo "  debug: $ENABLE_DEBUG"
echo "  nls: $ENABLE_NLS"

# Run meson setup
meson setup "$BUILD_DIR" "${MESON_ARGS[@]}"

echo ""
echo "Configuration complete. Now run 'make' to build."
echo "(Note: This will run 'ninja -C $BUILD_DIR')"
echo ""
echo "To install: make install"
CONFIGURE_EOF

chmod +x "$PROJECT_ROOT/configure"

# Create Makefile wrapper
cat > "$PROJECT_ROOT/Makefile" << 'MAKEFILE_EOF'
# Makefile - Autotools-compatible wrapper for Meson

BUILD_DIR = builddir

.PHONY: all install clean distclean check

all:
	@ninja -C $(BUILD_DIR)

install:
	@meson install -C $(BUILD_DIR)

clean:
	@if [ -d $(BUILD_DIR) ]; then ninja -C $(BUILD_DIR) clean; fi

distclean:
	@rm -rf $(BUILD_DIR) configure Makefile

check:
	@meson test -C $(BUILD_DIR) --print-errorlogs

# Convenience targets for Autotools compatibility
%:
	@ninja -C $(BUILD_DIR) $@
MAKEFILE_EOF

echo "Generated: configure, Makefile"
echo ""
echo "Backward compatibility files created."
echo "Users can now run: ./configure && make && make install"
