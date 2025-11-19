# Packaging Guide

This guide explains how to create distribution packages for MetaImGUI on various platforms.

## Table of Contents

- [Linux](#linux)
  - [Flatpak](#flatpak)
  - [Snap](#snap)
  - [AppImage](#appimage)
  - [.deb Package](#deb-package)
  - [.rpm Package](#rpm-package)
- [Windows](#windows)
  - [NSIS Installer](#nsis-installer)
  - [Portable ZIP](#portable-zip)
- [macOS](#macos)
  - [DMG](#dmg)
  - [App Bundle](#app-bundle)

---

## Linux

### Flatpak

Flatpak provides a sandboxed application with all dependencies bundled.

**Prerequisites:**
```bash
sudo apt-get install flatpak flatpak-builder
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak install flathub org.freedesktop.Platform//23.08 org.freedesktop.Sdk//23.08
```

**Build:**
```bash
cd packaging/flatpak

# Build the flatpak
flatpak-builder --force-clean --repo=repo build-dir com.metaimgui.MetaImGUI.yaml

# Create a bundle
flatpak build-bundle repo MetaImGUI.flatpak com.metaimgui.MetaImGUI

# Or install locally for testing
flatpak-builder --user --install --force-clean build-dir com.metaimgui.MetaImGUI.yaml
```

**Install:**
```bash
flatpak install MetaImGUI.flatpak
```

**Run:**
```bash
flatpak run com.metaimgui.MetaImGUI
```

**Uninstall:**
```bash
flatpak uninstall com.metaimgui.MetaImGUI
```

### Snap

Snap is Ubuntu's universal package format, also available on other distributions.

**Prerequisites:**
```bash
sudo apt-get install snapd snapcraft
```

**Build:**
```bash
cd packaging/snap
snapcraft

# Or with debug output
snapcraft --debug
```

This creates `metaimgui_1.0.0_amd64.snap`.

**Install:**
```bash
# Install locally
sudo snap install metaimgui_1.0.0_amd64.snap --dangerous

# Or from store (after publishing)
sudo snap install metaimgui
```

**Run:**
```bash
metaimgui
```

**Uninstall:**
```bash
sudo snap remove metaimgui
```

**Publishing to Snap Store:**
1. Create account at https://snapcraft.io
2. Login: `snapcraft login`
3. Upload: `snapcraft upload metaimgui_1.0.0_amd64.snap --release=stable`

### AppImage

AppImage provides a portable, single-file executable.

**Prerequisites:**
```bash
sudo apt-get install wget file
```

**Build:**
```bash
cd packaging
./create_linux_packages.sh 1.0.0
```

This uses `linuxdeploy` to create the AppImage.

**Run:**
```bash
chmod +x MetaImGUI-1.0.0-linux-x64.AppImage
./MetaImGUI-1.0.0-linux-x64.AppImage
```

### .deb Package

Debian/Ubuntu package format.

**Prerequisites:**
```bash
sudo apt-get install dpkg-dev
```

**Build:**
```bash
cd packaging
./create_linux_packages.sh 1.0.0
```

This creates `metaimgui_1.0.0_amd64.deb`.

**Install:**
```bash
sudo dpkg -i metaimgui_1.0.0_amd64.deb
sudo apt-get install -f  # Install dependencies
```

**Uninstall:**
```bash
sudo apt-get remove metaimgui
```

### .rpm Package

Red Hat/Fedora/openSUSE package format.

**Prerequisites:**
```bash
sudo dnf install rpm-build rpmdevtools
```

**Build:**
```bash
# Setup RPM build tree
rpmdev-setuptree

# Copy spec file
cp packaging/rpm/metaimgui.spec ~/rpmbuild/SPECS/

# Create source tarball
git archive --format=tar.gz --prefix=metaimgui-1.0.0/ HEAD > ~/rpmbuild/SOURCES/metaimgui-1.0.0.tar.gz

# Build
rpmbuild -ba ~/rpmbuild/SPECS/metaimgui.spec
```

**Install:**
```bash
sudo rpm -i ~/rpmbuild/RPMS/x86_64/metaimgui-1.0.0-1.x86_64.rpm
```

---

## Windows

### NSIS Installer

Creates a professional Windows installer with uninstaller.

**Prerequisites:**
- NSIS 3.x installed
- Built Windows executable

**Build:**
```powershell
cd packaging
.\create_windows_installer.ps1 -Version 1.0.0
```

This creates `MetaImGUI-1.0.0-windows-x64-installer.exe`.

**Install:**
- Run the installer
- Follow the wizard
- Creates Start Menu shortcuts
- Adds to Add/Remove Programs

**Uninstall:**
- Via Add/Remove Programs
- Or run uninstaller from installation directory

### Portable ZIP

Portable version that doesn't require installation.

**Build:**
```powershell
cd packaging
.\create_windows_installer.ps1 -Version 1.0.0
```

This also creates `MetaImGUI-1.0.0-windows-x64.zip`.

**Use:**
- Extract anywhere
- Run `MetaImGUI.exe`
- No installation required

---

## macOS

### DMG

Disk image for macOS distribution.

**Prerequisites:**
- macOS system
- Xcode Command Line Tools

**Build:**
```bash
cd packaging
./create_macos_dmg.sh 1.0.0
```

Or manually:
```bash
# Build app
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Create app bundle
mkdir -p MetaImGUI.app/Contents/MacOS
mkdir -p MetaImGUI.app/Contents/Resources
cp build/MetaImGUI MetaImGUI.app/Contents/MacOS/
cp Info.plist MetaImGUI.app/Contents/

# Create DMG
hdiutil create -volname "MetaImGUI" -srcfolder MetaImGUI.app -ov -format UDZO MetaImGUI-1.0.0-macos-x64.dmg
```

**Install:**
- Open DMG
- Drag app to Applications
- Eject DMG

### App Bundle

Standalone .app bundle.

**Structure:**
```
MetaImGUI.app/
├── Contents/
│   ├── Info.plist
│   ├── MacOS/
│   │   └── MetaImGUI
│   └── Resources/
│       ├── icon.icns
│       └── resources/
```

**Info.plist:**
```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN">
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
    <key>CFBundleShortVersionString</key>
    <string>1.0.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
</dict>
</plist>
```

---

## CI/CD Integration

All packaging is automated via GitHub Actions:

### Release Workflow

When you push a tag:
```bash
git tag v1.0.0
git push origin v1.0.0
```

GitHub Actions automatically:
1. Builds on Linux, Windows, macOS
2. Creates packages for each platform:
   - Linux: AppImage, .tar.gz
   - Windows: .zip
   - macOS: .dmg
3. Uploads to GitHub Releases
4. Generates checksums

### Manual Packaging

For testing or local distribution:

**Linux:**
```bash
./packaging/create_linux_packages.sh 1.0.0
```

**Windows:**
```powershell
.\packaging\create_windows_installer.ps1 -Version 1.0.0
```

**macOS:**
```bash
./packaging/create_macos_dmg.sh 1.0.0
```

---

## Package Testing

### Test Checklist

Before publishing packages:

- [ ] Install on fresh system
- [ ] Run application
- [ ] Check all features work
- [ ] Verify icon shows correctly
- [ ] Test desktop shortcuts
- [ ] Check uninstaller works
- [ ] Verify no missing dependencies
- [ ] Test on multiple OS versions

### Testing VMs

Recommended:
- **Linux**: Ubuntu 22.04, Fedora 39, Arch (latest)
- **Windows**: Windows 10, Windows 11
- **macOS**: macOS 12+

---

## Distribution

### GitHub Releases

Packages are automatically uploaded to GitHub Releases.

### Package Repositories

**Flathub:**
1. Fork https://github.com/flathub/flathub
2. Add your manifest
3. Submit PR

**Snap Store:**
```bash
snapcraft login
snapcraft upload metaimgui*.snap --release=stable
```

**AUR (Arch User Repository):**
Create PKGBUILD and submit to AUR.

**Homebrew:**
Create formula in homebrew-cask or your tap.

---

## Troubleshooting

### Flatpak Build Fails

```bash
# Clean build
flatpak-builder --force-clean build-dir com.metaimgui.MetaImGUI.yaml

# Check dependencies
flatpak list --runtime

# View build logs
flatpak-builder --verbose build-dir com.metaimgui.MetaImGUI.yaml
```

### Snap Build Fails

```bash
# Clean and retry
snapcraft clean
snapcraft --debug

# Shell into build environment
snapcraft --shell-after

# Check logs
journalctl -xe
```

### Missing Dependencies

**Linux:**
```bash
ldd build/MetaImGUI  # Check dynamic dependencies
```

**Windows:**
```powershell
# Use Dependency Walker or dumpbin
dumpbin /DEPENDENTS MetaImGUI.exe
```

**macOS:**
```bash
otool -L MetaImGUI.app/Contents/MacOS/MetaImGUI
```

---

## Resources

- [Flatpak Documentation](https://docs.flatpak.org/)
- [Snapcraft Documentation](https://snapcraft.io/docs)
- [AppImage Documentation](https://docs.appimage.org/)
- [NSIS Documentation](https://nsis.sourceforge.io/Docs/)
- [macOS Packaging Guide](https://developer.apple.com/library/archive/documentation/CoreFoundation/Conceptual/CFBundles/)

