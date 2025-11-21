#!/bin/bash
# Script to generate icon files from SVG source

set -e

echo "Generating MetaImGUI icons from SVG..."

# Check if ImageMagick is available
if ! command -v convert &> /dev/null; then
    echo "ImageMagick not found. Please install it:"
    echo "  Ubuntu/Debian: sudo apt-get install imagemagick"
    echo "  macOS: brew install imagemagick"
    echo "  Fedora: sudo dnf install ImageMagick"
    exit 1
fi

# Generate PNG files
echo "Generating PNG files..."
convert -background none metaimgui.svg -resize 16x16 metaimgui-16.png
convert -background none metaimgui.svg -resize 32x32 metaimgui-32.png
convert -background none metaimgui.svg -resize 48x48 metaimgui-48.png
convert -background none metaimgui.svg -resize 64x64 metaimgui-64.png
convert -background none metaimgui.svg -resize 128x128 metaimgui-128.png
convert -background none metaimgui.svg -resize 256x256 metaimgui-256.png
convert -background none metaimgui.svg -resize 512x512 metaimgui-512.png

echo "PNG files generated successfully!"

# Generate Windows .ico
echo "Generating Windows .ico..."
convert metaimgui-16.png metaimgui-32.png metaimgui-48.png metaimgui-256.png metaimgui.ico
echo "Windows .ico generated successfully!"

# Generate macOS .icns (macOS only)
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Generating macOS .icns..."

    mkdir -p metaimgui.iconset

    cp metaimgui-16.png metaimgui.iconset/icon_16x16.png
    cp metaimgui-32.png metaimgui.iconset/icon_16x16@2x.png
    cp metaimgui-32.png metaimgui.iconset/icon_32x32.png
    cp metaimgui-64.png metaimgui.iconset/icon_32x32@2x.png
    cp metaimgui-128.png metaimgui.iconset/icon_128x128.png
    cp metaimgui-256.png metaimgui.iconset/icon_128x128@2x.png
    cp metaimgui-256.png metaimgui.iconset/icon_256x256.png
    cp metaimgui-512.png metaimgui.iconset/icon_256x256@2x.png
    cp metaimgui-512.png metaimgui.iconset/icon_512x512.png

    # Generate 1024x1024 for retina
    convert -background none metaimgui.svg -resize 1024x1024 metaimgui-1024.png
    cp metaimgui-1024.png metaimgui.iconset/icon_512x512@2x.png

    iconutil -c icns metaimgui.iconset
    rm -rf metaimgui.iconset

    echo "macOS .icns generated successfully!"
else
    echo "Skipping .icns generation (macOS only)"
fi

echo ""
echo "Icon generation complete!"
echo "Generated files:"
echo "  - metaimgui-*.png (various sizes)"
echo "  - metaimgui.ico (Windows)"
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "  - metaimgui.icns (macOS)"
fi

