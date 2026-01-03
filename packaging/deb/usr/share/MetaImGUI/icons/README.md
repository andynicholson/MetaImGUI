# MetaImGUI Icons

This directory contains the application icons in various formats for different platforms.

## Icon Files

- `metaimgui.svg` - Vector source (scalable)
- `metaimgui-256.png` - 256x256 PNG for Linux (AppImage, .deb)
- `metaimgui.ico` - Windows icon bundle (16x16, 32x32, 48x48, 256x256)
- `metaimgui.icns` - macOS icon bundle

## Creating Icons

### From SVG to PNG

```bash
# Using Inkscape
inkscape metaimgui.svg --export-filename=metaimgui-256.png --export-width=256 --export-height=256

# Using ImageMagick
convert -background none metaimgui.svg -resize 256x256 metaimgui-256.png
```

### Creating Windows .ico

```bash
# Using ImageMagick
convert metaimgui.svg -define icon:auto-resize=256,128,96,64,48,32,16 metaimgui.ico

# Or using online tools like:
# - https://convertio.co/svg-ico/
# - https://cloudconvert.com/svg-to-ico
```

### Creating macOS .icns

```bash
# Create icon set directory
mkdir metaimgui.iconset

# Generate all required sizes
sips -z 16 16     metaimgui-256.png --out metaimgui.iconset/icon_16x16.png
sips -z 32 32     metaimgui-256.png --out metaimgui.iconset/icon_16x16@2x.png
sips -z 32 32     metaimgui-256.png --out metaimgui.iconset/icon_32x32.png
sips -z 64 64     metaimgui-256.png --out metaimgui.iconset/icon_32x32@2x.png
sips -z 128 128   metaimgui-256.png --out metaimgui.iconset/icon_128x128.png
sips -z 256 256   metaimgui-256.png --out metaimgui.iconset/icon_128x128@2x.png
sips -z 256 256   metaimgui-256.png --out metaimgui.iconset/icon_256x256.png
sips -z 512 512   metaimgui-256.png --out metaimgui.iconset/icon_256x256@2x.png
sips -z 512 512   metaimgui-256.png --out metaimgui.iconset/icon_512x512.png
sips -z 1024 1024 metaimgui-256.png --out metaimgui.iconset/icon_512x512@2x.png

# Create .icns
iconutil -c icns metaimgui.iconset
```

## Design Guidelines

The MetaImGUI icon should:
- Be simple and recognizable at small sizes
- Work well on both light and dark backgrounds
- Represent the "meta" nature of the project (template/framework)
- Be professional and modern

### Suggested Design Concepts

1. **Layered Squares**: Overlapping squares representing layers of abstraction
2. **Window Frame**: Simple window outline with "M" or gear inside
3. **Puzzle Piece**: Representing template/framework that fits into projects
4. **Abstract "M"**: Stylized letter M with modern geometric style

## Current Icon

The current placeholder icon is a simple geometric design. Feel free to replace with a more polished design that fits your project's identity.

## License

Icons should be created by you or sourced from:
- Public domain
- Creative Commons (with attribution)
- Custom commissioned work

Document the icon source and license here if using third-party artwork.

