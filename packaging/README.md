# MetaImGUI Packaging

This directory contains scripts and configurations for creating distribution packages for MetaImGUI.

## Linux Packages

### AppImage

AppImages are portable, self-contained applications that run on most Linux distributions.

**Build:**
```bash
./create_linux_packages.sh 1.0.0
```

The AppImage will be created in `packaging/output/`.

**Run:**
```bash
chmod +x MetaImGUI-1.0.0-linux-x64.AppImage
./MetaImGUI-1.0.0-linux-x64.AppImage
```

### Debian Package (.deb)

For Ubuntu, Debian, and derivatives.

**Build:**
```bash
./create_linux_packages.sh 1.0.0
```

**Install:**
```bash
sudo dpkg -i packaging/output/metaimgui_1.0.0_amd64.deb
sudo apt-get install -f  # Install dependencies
```

### Flatpak

Flatpak provides sandboxed applications with modern runtimes.

**Build:**
```bash
cd packaging/flatpak
flatpak-builder --force-clean build-dir com.metaimgui.MetaImGUI.yaml
flatpak-builder --repo=repo --force-clean build-dir com.metaimgui.MetaImGUI.yaml
flatpak build-bundle repo metaimgui.flatpak com.metaimgui.MetaImGUI
```

**Install:**
```bash
flatpak install metaimgui.flatpak
```

**Run:**
```bash
flatpak run com.metaimgui.MetaImGUI
```

### Snap

Snaps work across many Linux distributions.

**Build:**
```bash
cd packaging/snap
snapcraft
```

**Install:**
```bash
sudo snap install metaimgui_1.0.0_amd64.snap --dangerous
```

**Run:**
```bash
metaimgui
```

## Windows Packages

### NSIS Installer

Creates a professional Windows installer.

**Requirements:**
- NSIS 3.0 or later
- vcpkg with glfw3 and curl installed
- Visual Studio 2019 or later with C++ tools

**Build:**
```powershell
# Set VCPKG_ROOT if not at C:\vcpkg
$env:VCPKG_ROOT = "C:\your\vcpkg\path"

.\packaging\create_windows_installer.ps1 -Version 1.0.0
```

**Output:**
- `MetaImGUI-1.0.0-Setup.exe` - Installer with GLFW and libcurl DLLs
- `MetaImGUI-1.0.0-windows-x64-portable.zip` - Portable version

**Note:** Users may need Visual C++ Redistributable 2015-2022 installed.

### WiX Toolset (MSI)

For enterprise deployments requiring MSI format.

**Requirements:**
- Install WiX Toolset from https://wixtoolset.org/

**Build:**
```powershell
# TODO: Add WiX configuration
```

## macOS Packages

### DMG Disk Image

**Build:**
The GitHub Actions release workflow automatically creates DMG files.

Manual build:
```bash
# Create app bundle
mkdir -p MetaImGUI.app/Contents/MacOS
mkdir -p MetaImGUI.app/Contents/Resources
cp build/MetaImGUI MetaImGUI.app/Contents/MacOS/
cp -r resources MetaImGUI.app/Contents/Resources/

# Create Info.plist
cat > MetaImGUI.app/Contents/Info.plist << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>MetaImGUI</string>
    <key>CFBundleIdentifier</key>
    <string>com.metaimgui.app</string>
    <key>CFBundleName</key>
    <string>MetaImGUI</string>
    <key>CFBundleVersion</key>
    <string>1.0.0</string>
</dict>
</plist>
EOF

# Create DMG
hdiutil create -volname "MetaImGUI" -srcfolder MetaImGUI.app -ov -format UDZO MetaImGUI-1.0.0.dmg
```

## Automated Packaging

### GitHub Actions

The project includes automated packaging via GitHub Actions:

1. **CI Workflow** (`.github/workflows/ci.yml`):
   - Builds on every push/PR
   - Runs tests
   - Creates artifacts

2. **Release Workflow** (`.github/workflows/release.yml`):
   - Triggers on version tags (v*.*.*)
   - Creates packages for all platforms
   - Publishes GitHub Release
   - Uploads all artifacts

**Trigger a release:**
```bash
git tag v1.0.0
git push origin v1.0.0
```

## Package Metadata

### Icons

Icons should be placed in `resources/icons/`:
- `metaimgui-16.png` - 16x16
- `metaimgui-32.png` - 32x32
- `metaimgui-48.png` - 48x48
- `metaimgui-64.png` - 64x64
- `metaimgui-128.png` - 128x128
- `metaimgui-256.png` - 256x256
- `metaimgui-512.png` - 512x512
- `metaimgui.ico` - Windows icon
- `metaimgui.icns` - macOS icon
- `metaimgui.svg` - Source vector

Generate icons using:
```bash
cd resources/icons
./generate_icons.sh
```

### Desktop Files

Desktop entry for Linux (Flatpak/Snap):
```ini
[Desktop Entry]
Type=Application
Name=MetaImGUI
GenericName=ImGui Application Template
Comment=Cross-platform ImGui application template
Icon=metaimgui
Exec=MetaImGUI
Terminal=false
Categories=Development;IDE;
```

## Distribution Channels

### Flathub

To publish on Flathub:
1. Fork https://github.com/flathub/flathub
2. Create PR with your manifest
3. Follow Flathub submission guidelines

### Snap Store

To publish on Snap Store:
```bash
snapcraft login
snapcraft upload metaimgui_1.0.0_amd64.snap
snapcraft release metaimgui <revision> stable
```

### Microsoft Store

For Microsoft Store:
1. Create app package using Visual Studio
2. Submit via Partner Center
3. Follow Microsoft Store guidelines

### Mac App Store

For Mac App Store:
1. Sign with Apple Developer certificate
2. Create app archive
3. Submit via App Store Connect

## Package Testing

### Test Matrix

Test all packages on:
- [ ] Ubuntu 20.04 LTS
- [ ] Ubuntu 22.04 LTS
- [ ] Debian 11
- [ ] Fedora 38
- [ ] Windows 10
- [ ] Windows 11
- [ ] macOS 12 (Monterey)
- [ ] macOS 13 (Ventura)
- [ ] macOS 14 (Sonoma)

### Verification Checklist

For each package:
- [ ] Application launches successfully
- [ ] All features work correctly
- [ ] Icons display properly
- [ ] Menu items function
- [ ] Settings are saved/loaded
- [ ] Update checker works
- [ ] No missing dependencies
- [ ] Uninstall works cleanly

## Troubleshooting

### AppImage Issues

**"Cannot execute binary file"**
```bash
chmod +x MetaImGUI-*.AppImage
```

**Missing FUSE**
```bash
sudo apt-get install fuse
# Or extract and run:
./MetaImGUI-*.AppImage --appimage-extract
./squashfs-root/AppRun
```

### Flatpak Issues

**Build fails**
```bash
flatpak-builder --force-clean build-dir com.metaimgui.MetaImGUI.yaml
```

**Runtime not found**
```bash
flatpak install flathub org.freedesktop.Platform//23.08
flatpak install flathub org.freedesktop.Sdk//23.08
```

### Snap Issues

**Build fails**
```bash
snapcraft clean
snapcraft
```

**Classic confinement needed**
If the app needs more permissions, change to classic confinement in snapcraft.yaml.

## Resources

- [AppImage Documentation](https://docs.appimage.org/)
- [Flatpak Documentation](https://docs.flatpak.org/)
- [Snap Documentation](https://snapcraft.io/docs)
- [NSIS Documentation](https://nsis.sourceforge.io/Docs/)
- [WiX Toolset](https://wixtoolset.org/)
- [macOS Packaging](https://developer.apple.com/documentation/xcode/distributing-your-app-for-beta-testing-and-releases)

## Testing Packages

For detailed testing instructions on each platform, see [TESTING_GUIDE.md](TESTING_GUIDE.md).

Quick tests:
- **Linux**: `./packaging/test_packages.sh 1.0.0`
- **Windows**: `.\packaging\create_windows_installer.ps1 -Version 1.0.0`
- **macOS**: Build and create DMG as shown in TESTING_GUIDE.md

## Questions?

See [CONTRIBUTING.md](../CONTRIBUTING.md) or open an issue on GitHub.

