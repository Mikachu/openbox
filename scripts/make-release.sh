#!/bin/bash
# Openbox 4.0.1 Release Script
# Creates source tarball and release artifacts

set -e

VERSION="4.0.1"
TAG="v${VERSION}"
REPO="https://github.com/openbox/openbox"

echo "=========================================="
echo "Openbox ${VERSION} Release Script"
echo "=========================================="
echo ""

# Check we're in the right directory
if [ ! -f "meson.build" ]; then
    echo "Error: must run from Openbox source root"
    exit 1
fi

# Verify version
if ! grep -q "version : '${VERSION}'" meson.build; then
    echo "Error: version mismatch in meson.build"
    exit 1
fi

# Create release directory
RELEASE_DIR="openbox-${VERSION}"
mkdir -p "${RELEASE_DIR}"

echo "Preparing release files..."

# Copy source files
cp -r \
    src \
    theme \
    python \
    extension \
    portal \
    include \
    data \
    doc \
    docs \
    themes \
    systemd \
    debian \
    rpm \
    flatpak \
    tests \
    scripts \
    meson.build \
    meson_options.txt \
    VERSION \
    NEWS \
    README.md \
    LICENSE \
    AUTHORS \
    CHANGELOG.md \
    "${RELEASE_DIR}/"

# Generate configure script (for compatibility)
cat > "${RELEASE_DIR}/configure" << 'EOF'
#!/bin/bash
# Compatibility configure script for Openbox 4.0.1
echo "Openbox 4.0.1 uses Meson build system."
echo ""
echo "To build Openbox:"
echo "  meson setup build"
echo "  ninja -C build"
echo "  sudo ninja -C build install"
echo ""
echo "For detailed instructions, see README.md"
EOF
chmod +x "${RELEASE_DIR}/configure"

# Create tarball
echo "Creating tarball..."
TARBALL="openbox-${VERSION}.tar.gz"
tar czf "${TARBALL}" "${RELEASE_DIR}"

# Generate checksums
echo "Generating checksums..."
sha256sum "${TARBALL}" > "${TARBALL}.sha256"
md5sum "${TARBALL}" > "${TARBALL}.md5"

# Create detached signature (if gpg available)
if command -v gpg &> /dev/null; then
    echo "Creating GPG signature..."
    gpg --armor --detach-sign "${TARBALL}"
fi

# Create release notes snippet
cat > "openbox-${VERSION}-release-notes.txt" << EOF
Openbox ${VERSION} Release Notes
================================

$(head -50 CHANGELOG.md | grep -A 50 "## \\[${VERSION}\\]")

Full changelog: https://github.com/openbox/openbox/blob/${TAG}/CHANGELOG.md

Installation
------------

From source:
  tar xzf ${TARBALL}
  cd ${RELEASE_DIR}
  meson setup build
  ninja -C build
  sudo ninja -C build install

Packages will be available shortly for major distributions.

Verification
------------

SHA256: $(cat "${TARBALL}.sha256" | cut -d' ' -f1)

GPG signature: ${TARBALL}.asc (if available)

EOF

# Cleanup
rm -rf "${RELEASE_DIR}"

echo ""
echo "=========================================="
echo "Release artifacts created:"
echo "=========================================="
ls -lh openbox-${VERSION}*

echo ""
echo "Next steps:"
echo "1. Upload ${TARBALL} to GitHub releases"
echo "2. Upload checksums and signatures"
echo "3. Create Git tag: git tag -a ${TAG} -m 'Release ${VERSION}'"
echo "4. Push tag: git push origin ${TAG}"
echo "5. Update website and announce"
echo ""
echo "Done!"
