#!/bin/bash
# macOS DMG Creation Script for MetaImGUI
# Creates a distributable DMG file with proper app bundle

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
VERSION=${1:-"1.0.0"}
BUILD_DIR="build"
PACKAGE_DIR="packaging/output"
APP_NAME="MetaImGUI.app"
DMG_NAME="${PROJECT_NAME}-${VERSION}-macOS"

print_info "Creating macOS DMG for $PROJECT_NAME v$VERSION"

# Ensure we're running on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    print_error "This script must be run on macOS"
    exit 1
fi

# Ensure we're in the project root
if [ ! -f "CMakeLists.txt" ]; then
    print_error "Please run this script from the project root directory"
    exit 1
fi

# Create output directory
mkdir -p "$PACKAGE_DIR"

# Build the project if not already built
if [ ! -d "$BUILD_DIR/$APP_NAME" ]; then
    print_info "Building project..."
    cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$BUILD_DIR" --config Release --parallel
fi

print_success "Build complete"

# Verify app bundle exists
if [ ! -d "$BUILD_DIR/$APP_NAME" ]; then
    print_error "App bundle not found at $BUILD_DIR/$APP_NAME"
    exit 1
fi

# Verify resources are in the bundle
if [ ! -f "$BUILD_DIR/$APP_NAME/Contents/Resources/resources/translations/translations.json" ]; then
    print_error "translations.json not found in app bundle!"
    print_info "Copying resources to bundle..."
    mkdir -p "$BUILD_DIR/$APP_NAME/Contents/Resources"
    cp -r resources "$BUILD_DIR/$APP_NAME/Contents/Resources/"
fi

print_success "App bundle verified"

# Create DMG staging directory
DMG_STAGING="packaging/dmg_staging"
rm -rf "$DMG_STAGING"
mkdir -p "$DMG_STAGING"

# Copy app bundle to staging
cp -R "$BUILD_DIR/$APP_NAME" "$DMG_STAGING/"

# Create symbolic link to Applications folder
ln -s /Applications "$DMG_STAGING/Applications"

# Copy README if exists
if [ -f "README.md" ]; then
    cp README.md "$DMG_STAGING/"
fi

print_info "Creating DMG..."

# Create temporary DMG
TEMP_DMG="$PACKAGE_DIR/temp_${DMG_NAME}.dmg"
FINAL_DMG="$PACKAGE_DIR/${DMG_NAME}.dmg"

# Remove old DMG files
rm -f "$TEMP_DMG" "$FINAL_DMG"

# Create DMG with staging directory
# Calculate size needed (add 50MB buffer)
DMG_SIZE=$(du -sm "$DMG_STAGING" | cut -f1)
DMG_SIZE=$((DMG_SIZE + 50))

hdiutil create -volname "$PROJECT_NAME" \
    -srcfolder "$DMG_STAGING" \
    -ov -format UDRW \
    -size ${DMG_SIZE}m \
    "$TEMP_DMG"

print_info "Mounting DMG for customization..."

# Mount the DMG
MOUNT_DIR="/Volumes/$PROJECT_NAME"
hdiutil attach "$TEMP_DMG" -mountpoint "$MOUNT_DIR"

# Wait for mount
sleep 2

# Customize DMG appearance (requires AppleScript)
print_info "Customizing DMG appearance..."

osascript <<EOF
tell application "Finder"
    tell disk "$PROJECT_NAME"
        open
        set current view of container window to icon view
        set toolbar visible of container window to false
        set statusbar visible of container window to false
        set the bounds of container window to {100, 100, 700, 450}

        set viewOptions to the icon view options of container window
        set arrangement of viewOptions to not arranged
        set icon size of viewOptions to 128

        -- Position icons
        set position of item "$APP_NAME" of container window to {150, 150}
        set position of item "Applications" of container window to {450, 150}

        update without registering applications
        delay 2
    end tell
end tell
EOF

# Unmount the DMG
hdiutil detach "$MOUNT_DIR"

print_info "Converting to compressed DMG..."

# Convert to compressed, read-only DMG
hdiutil convert "$TEMP_DMG" \
    -format UDZO \
    -o "$FINAL_DMG"

# Clean up
rm -f "$TEMP_DMG"
rm -rf "$DMG_STAGING"

print_success "DMG created: $FINAL_DMG"

# Show file info
echo ""
echo "DMG file information:"
ls -lh "$FINAL_DMG"
echo ""

print_success "macOS packaging complete!"
echo ""
echo "Install by:"
echo "  1. Mount the DMG: open $FINAL_DMG"
echo "  2. Drag MetaImGUI.app to Applications folder"
echo ""

