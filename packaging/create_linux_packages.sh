#!/bin/bash
# Linux Package Creation Script for MetaImGUI
# Creates AppImage and .deb packages

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# Configuration
PROJECT_NAME="MetaImGUI"
APP_NAME="MetaImGUI"
VERSION=${1:-"1.0.0"}
ARCH="x86_64"
BUILD_DIR="build"
PACKAGE_DIR="packaging/output"

print_info "Creating Linux packages for $PROJECT_NAME v$VERSION"

# Ensure we're in the project root
if [ ! -f "CMakeLists.txt" ]; then
    print_error "Please run this script from the project root directory"
    exit 1
fi

# Create output directory
mkdir -p "$PACKAGE_DIR"

# Build the project if not already built
if [ ! -f "$BUILD_DIR/$PROJECT_NAME" ]; then
    print_info "Building project..."
    cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$BUILD_DIR" --config Release --parallel
fi

print_success "Build complete"

# ============================================================================
# Create AppImage
# ============================================================================
print_info "Creating AppImage..."

# Download linuxdeploy if not present
if [ ! -f "packaging/linuxdeploy-x86_64.AppImage" ]; then
    print_info "Downloading linuxdeploy..."
    wget -q -O packaging/linuxdeploy-x86_64.AppImage \
        https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x packaging/linuxdeploy-x86_64.AppImage
fi

# Create AppDir structure
APPDIR="packaging/AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/lib"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"
mkdir -p "$APPDIR/usr/share/metainfo"

# Copy executable
cp "$BUILD_DIR/$PROJECT_NAME" "$APPDIR/usr/bin/"

# Copy resources if they exist
if [ -d "resources" ]; then
    mkdir -p "$APPDIR/usr/share/$PROJECT_NAME"
    cp -r resources/* "$APPDIR/usr/share/$PROJECT_NAME/"
fi

# Create desktop entry
cat > "$APPDIR/usr/share/applications/${APP_NAME}.desktop" << EOF
[Desktop Entry]
Type=Application
Name=$APP_NAME
Comment=ImGui Application Template
Exec=$PROJECT_NAME
Icon=${APP_NAME}
Categories=Development;
Terminal=false
EOF

# Create AppStream metadata
cat > "$APPDIR/usr/share/metainfo/${APP_NAME}.appdata.xml" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop">
  <id>${APP_NAME}</id>
  <name>$APP_NAME</name>
  <summary>ImGui Application Template</summary>
  <metadata_license>CC0-1.0</metadata_license>
  <project_license>MIT</project_license>
  <description>
    <p>A template for creating ImGui-based applications with modern C++20.</p>
  </description>
</component>
EOF

# Copy icon file
if [ -f "resources/icons/metaimgui-256.png" ]; then
    cp "resources/icons/metaimgui-256.png" "$APPDIR/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png"
    print_info "Using application icon"
elif [ ! -f "$APPDIR/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png" ]; then
    # Create a simple colored square as placeholder if no icon exists
    if command -v convert &> /dev/null; then
        convert -size 256x256 xc:#3498db "$APPDIR/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png"
        print_info "Generated placeholder icon"
    else
        print_error "Icon not found and ImageMagick not available to generate placeholder"
        print_info "Run: cd resources/icons && ./generate_icons.sh"
        exit 1
    fi
fi

# Build AppImage
cd packaging
export OUTPUT="${PROJECT_NAME}-${VERSION}-${ARCH}.AppImage"
./linuxdeploy-x86_64.AppImage --appdir AppDir --output appimage 2>/dev/null || true

# Move to output directory
if [ -f "$OUTPUT" ]; then
    mv "$OUTPUT" "output/"
    print_success "AppImage created: $PACKAGE_DIR/$OUTPUT"
else
    print_error "AppImage creation failed"
fi
cd ..

# ============================================================================
# Create .deb package
# ============================================================================
print_info "Creating .deb package..."

DEB_DIR="packaging/deb"
rm -rf "$DEB_DIR"
mkdir -p "$DEB_DIR/DEBIAN"
mkdir -p "$DEB_DIR/usr/bin"
mkdir -p "$DEB_DIR/usr/share/applications"
mkdir -p "$DEB_DIR/usr/share/icons/hicolor/256x256/apps"
mkdir -p "$DEB_DIR/usr/share/doc/$PROJECT_NAME"

# Copy files
cp "$BUILD_DIR/$PROJECT_NAME" "$DEB_DIR/usr/bin/"
cp "$APPDIR/usr/share/applications/${APP_NAME}.desktop" "$DEB_DIR/usr/share/applications/"

# Copy icon
if [ -f "resources/icons/metaimgui-256.png" ]; then
    cp "resources/icons/metaimgui-256.png" "$DEB_DIR/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png"
else
    cp "$APPDIR/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png" "$DEB_DIR/usr/share/icons/hicolor/256x256/apps/"
fi

# Copy resources
if [ -d "resources" ]; then
    mkdir -p "$DEB_DIR/usr/share/$PROJECT_NAME"
    cp -r resources/* "$DEB_DIR/usr/share/$PROJECT_NAME/"
fi

# Create copyright file
cat > "$DEB_DIR/usr/share/doc/$PROJECT_NAME/copyright" << EOF
Format: https://www.debian.org/doc/packaging-manuals/copyright-format/1.0/
Upstream-Name: $PROJECT_NAME
Source: https://github.com/yourusername/$PROJECT_NAME

Files: *
Copyright: $(date +%Y) Your Name
License: MIT
EOF

# Create control file
INSTALLED_SIZE=$(du -sk "$DEB_DIR/usr" | cut -f1)
cat > "$DEB_DIR/DEBIAN/control" << EOF
Package: ${PROJECT_NAME,,}
Version: $VERSION
Section: devel
Priority: optional
Architecture: amd64
Depends: libglfw3, libgl1, libc6
Maintainer: Your Name <your.email@example.com>
Installed-Size: $INSTALLED_SIZE
Description: ImGui Application Template
 A template for creating ImGui-based applications with modern C++20,
 cross-platform build system, and automated CI/CD.
EOF

# Build .deb package
DEB_FILE="${PROJECT_NAME,,}_${VERSION}_amd64.deb"
dpkg-deb --build "$DEB_DIR" "$PACKAGE_DIR/$DEB_FILE" 2>/dev/null || {
    print_error ".deb creation requires dpkg-deb"
}

if [ -f "$PACKAGE_DIR/$DEB_FILE" ]; then
    print_success ".deb package created: $PACKAGE_DIR/$DEB_FILE"
fi

# ============================================================================
# Create tarball
# ============================================================================
print_info "Creating tarball..."

TARBALL_DIR="$PROJECT_NAME-$VERSION"
rm -rf "packaging/$TARBALL_DIR"
mkdir -p "packaging/$TARBALL_DIR"

cp "$BUILD_DIR/$PROJECT_NAME" "packaging/$TARBALL_DIR/"
cp README.md "packaging/$TARBALL_DIR/" 2>/dev/null || true
if [ -d "resources" ]; then
    cp -r resources "packaging/$TARBALL_DIR/"
fi

cd packaging
tar -czf "output/${PROJECT_NAME}-${VERSION}-linux-x64.tar.gz" "$TARBALL_DIR"
cd ..

print_success "Tarball created: $PACKAGE_DIR/${PROJECT_NAME}-${VERSION}-linux-x64.tar.gz"

# ============================================================================
# Summary
# ============================================================================
echo ""
print_success "Package creation complete!"
echo ""
echo "Created packages:"
ls -lh "$PACKAGE_DIR"
echo ""
echo "Install .deb with: sudo dpkg -i $PACKAGE_DIR/$DEB_FILE"
echo "Run AppImage with: ./$PACKAGE_DIR/${PROJECT_NAME}-${VERSION}-${ARCH}.AppImage"
echo "Extract tarball with: tar -xzf $PACKAGE_DIR/${PROJECT_NAME}-${VERSION}-linux-x64.tar.gz"

