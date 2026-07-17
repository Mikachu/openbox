#!/bin/bash
# Build script for Openbox AppImage
# Creates a universal Linux binary that runs on most distributions

set -e

APP=\"Openbox\"
LOWERAPP=\"openbox\"
VERSION=\"4.0.0\"
ARCH=\"x86_64\"

# Working directories
BUILD_DIR=\"$(pwd)/build-appimage\"
APP_DIR=\"$BUILD_DIR/$APP.AppDir\"
OUT_DIR=\"$(pwd)/dist\"

# Clean previous builds
rm -rf \"$BUILD_DIR\"
mkdir -p \"$APP_DIR/usr/bin\" \"$APP_DIR/usr/lib\" \"$APP_DIR/usr/share\"
mkdir -p \"$OUT_DIR\"

echo \"Building Openbox AppImage $VERSION...\"

# Build Openbox with static linking where possible
meson setup \"$BUILD_DIR/build\" \
    --prefix=/usr \
    --buildtype=release \
    -Db_lto=true \
    -Dportal=true \
    -Dextensions=true

ninja -C \"$BUILD_DIR/build\"

# Install to AppDir
DESTDIR=\"$APP_DIR\" ninja -C \"$BUILD_DIR/build\" install

# Copy dependencies
copy_deps() {
    local binary=\"$1\"
    local target_dir=\"$2\"
    
    ldd \"$binary\" | grep \"=> /\" | awk '{print $3}' | while read -r lib; do
        if [[ \"$lib\" == /lib* ]] || [[ \"$lib\" == /usr/lib* ]]; then
            if [ ! -f \"$target_dir/$(basename $lib)\" ]; then
                cp -v \"$lib\" \"$target_dir/\" 2>/dev/null || true
            fi
        fi
    done
}

# Copy Openbox binary deps
copy_deps \"$APP_DIR/usr/bin/openbox\" \"$APP_DIR/usr/lib\"

# Copy additional libraries
for lib in libX11.so.6 libXinerama.so.1 libXrandr.so.2 libXcursor.so.1 \
           libXft.so.2 libxml2.so.2 libglib-2.0.so.0 libcairo.so.2 \
           libpango-1.0.so.0 libstartup-notification-1.so.0; do
    if [ -f \"/lib/x86_64-linux-gnu/$lib\" ]; then
        cp \"/lib/x86_64-linux-gnu/$lib\" \"$APP_DIR/usr/lib/\" 2>/dev/null || true
    elif [ -f \"/usr/lib/x86_64-linux-gnu/$lib\" ]; then
        cp \"/usr/lib/x86_64-linux-gnu/$lib\" \"$APP_DIR/usr/lib/\" 2>/dev/null || true
    fi
done

# Create AppRun script (entry point)
cat > \"$APP_DIR/AppRun\" << 'EOF'
#!/bin/bash
APPDIR=\"$(dirname \"$(readlink -f \"$0\")\")\"
export PATH=\"$APPDIR/usr/bin:$PATH\"
export LD_LIBRARY_PATH=\"$APPDIR/usr/lib:$LD_LIBRARY_PATH\"
export XDG_DATA_DIRS=\"$APPDIR/usr/share:$XDG_DATA_DIRS\"

# Set default config if not present
if [ -z \"$XDG_CONFIG_HOME\" ]; then
    export XDG_CONFIG_HOME=\"$HOME/.config\"
fi

mkdir -p \"$XDG_CONFIG_HOME/openbox\"

# Copy default configs if not present
if [ ! -f \"$XDG_CONFIG_HOME/openbox/rc.xml\" ]; then
    cp \"$APPDIR/usr/share/openbox/rc.xml\" \"$XDG_CONFIG_HOME/openbox/\" 2>/dev/null || true
fi

exec \"$APPDIR/usr/bin/openbox\" \"$@\"
EOF

chmod +x \"$APP_DIR/AppRun\"

# Create desktop file
cat > \"$APP_DIR/$LOWERAPP.desktop\" << EOF
[Desktop Entry]
Name=$APP
Exec=openbox
Icon=openbox
Type=Application
Categories=System;WindowManager;
Comment=Highly configurable window manager
EOF

# Copy icon
if [ -f \"data/icons/openbox.svg\" ]; then
    cp \"data/icons/openbox.svg\" \"$APP_DIR/openbox.svg\"
elif [ -f \"$APP_DIR/usr/share/icons/hicolor/scalable/apps/openbox.svg\" ]; then
    cp \"$APP_DIR/usr/share/icons/hicolor/scalable/apps/openbox.svg\" \"$APP_DIR/openbox.svg\"
fi

# Download AppImageTool if not present
APPIMAGETOOL=\"$BUILD_DIR/appimagetool-x86_64.AppImage\"
if [ ! -f \"$APPIMAGETOOL\" ]; then
    wget -q \"https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage\" -O \"$APPIMAGETOOL\"
    chmod +x \"$APPIMAGETOOL\"
fi

# Build AppImage
echo \"Creating AppImage...\"
ARCH=\"$ARCH\" \"$APPIMAGETOOL\" \"$APP_DIR\" \"$OUT_DIR/Openbox-${VERSION}-${ARCH}.AppImage\"

echo \"AppImage created: $OUT_DIR/Openbox-${VERSION}-${ARCH}.AppImage\"

# Create update information for AppImageUpdate
echo \"zsync|https://openbox.org/releases/Openbox-${VERSION}-${ARCH}.AppImage.zsync\" > \"$OUT_DIR/Openbox-${VERSION}-${ARCH}.AppImage.zsync\"

# Generate checksums
cd \"$OUT_DIR\"
sha256sum \"Openbox-${VERSION}-${ARCH}.AppImage\" > \"Openbox-${VERSION}-${ARCH}.AppImage.sha256\"
cd -

echo \"Build complete!\"
