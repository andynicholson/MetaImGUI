# Packaging Testing Guide

This guide explains how to test each packaging format on their respective operating systems.

---

## Windows Testing

### Prerequisites

1. **Install NSIS** (for installer creation)
   ```powershell
   # Download from https://nsis.sourceforge.io/Download
   # Or use Chocolatey:
   choco install nsis
   ```

2. **Install vcpkg** (for dependencies)
   ```powershell
   # Clone vcpkg
   git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
   cd C:\vcpkg
   .\bootstrap-vcpkg.bat

   # Set environment variable (optional but recommended)
   [Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "User")
   ```

3. **Install dependencies via vcpkg**
   ```powershell
   cd C:\vcpkg
   .\vcpkg install glfw3:x64-windows curl:x64-windows
   ```

4. **Install Visual Studio 2019 or later** with C++ tools
   - Download from https://visualstudio.microsoft.com/downloads/
   - Select "Desktop development with C++" workload

### Building and Testing

#### Test 1: Build the Application

```powershell
# From project root
cd MetaImGUI

# Build with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release --parallel

# Verify executable was created
ls build\Release\MetaImGUI.exe
```

#### Test 2: Create Portable ZIP

```powershell
# Run packaging script
.\packaging\create_windows_installer.ps1 -Version 1.0.0

# Verify output
ls packaging\output\MetaImGUI-1.0.0-windows-x64-portable.zip

# Extract and test
Expand-Archive -Path packaging\output\MetaImGUI-1.0.0-windows-x64-portable.zip -DestinationPath test_portable
cd test_portable\MetaImGUI-1.0.0-portable
.\MetaImGUI.exe
```

**Verification checklist:**
- [ ] Application launches without errors
- [ ] No missing DLL errors (glfw3.dll, libcurl.dll should be present)
- [ ] UI renders correctly
- [ ] Update check works (may show no updates available)
- [ ] Settings are saved/loaded

#### Test 3: Create NSIS Installer

```powershell
# The create_windows_installer.ps1 script also creates the installer
.\packaging\create_windows_installer.ps1 -Version 1.0.0

# Verify installer was created
ls packaging\output\MetaImGUI-1.0.0-Setup.exe

# Install the application
.\packaging\output\MetaImGUI-1.0.0-Setup.exe
```

**Installation verification:**
- [ ] Installer runs and shows version number correctly
- [ ] Can choose installation directory
- [ ] Start Menu shortcuts are created
- [ ] Desktop shortcut is created (if selected)
- [ ] Application appears in "Add or Remove Programs"

**Runtime verification:**
- [ ] Launch from Start Menu works
- [ ] Desktop shortcut works
- [ ] Application runs correctly
- [ ] All features work

**Uninstallation verification:**
- [ ] Uninstaller appears in Start Menu
- [ ] Uninstall from "Add or Remove Programs" works
- [ ] All files and shortcuts are removed
- [ ] Installation directory is cleaned up

### Testing Without vcpkg at Default Location

If vcpkg is installed elsewhere:

```powershell
# Set VCPKG_ROOT environment variable
$env:VCPKG_ROOT = "D:\your\vcpkg\path"

# Then run packaging script
.\packaging\create_windows_installer.ps1 -Version 1.0.0
```

### Troubleshooting Windows

**Issue: "libcurl.dll not found"**
- Solution: Ensure vcpkg installed curl: `vcpkg install curl:x64-windows`
- Verify: Check `C:\vcpkg\installed\x64-windows\bin\libcurl.dll` exists

**Issue: "Application won't start on clean Windows"**
- Solution: Install Visual C++ Redistributable 2015-2022
- Download: https://aka.ms/vs/17/release/vc_redist.x64.exe

**Issue: "NSIS not found"**
- Solution: Ensure NSIS is installed and in PATH
- Check: `makensis /VERSION` should work

---

## macOS Testing

### Prerequisites

1. **Xcode Command Line Tools**
   ```bash
   xcode-select --install
   ```

2. **Homebrew** (package manager)
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

3. **Install dependencies**
   ```bash
   brew install cmake glfw llvm
   ```

### Building and Testing

#### Test 1: Build the Application

```bash
# From project root
cd MetaImGUI

# Setup dependencies
./setup_dependencies.sh

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Test executable
./build/MetaImGUI
```

#### Test 2: Create App Bundle

```bash
# Create app bundle structure
mkdir -p MetaImGUI.app/Contents/MacOS
mkdir -p MetaImGUI.app/Contents/Resources

# Copy executable
cp build/MetaImGUI MetaImGUI.app/Contents/MacOS/

# Copy resources
cp -r resources MetaImGUI.app/Contents/Resources/

# Generate icon (optional)
cd resources/icons
./generate_icons.sh
cd ../..
if [ -f resources/icons/metaimgui.icns ]; then
  cp resources/icons/metaimgui.icns MetaImGUI.app/Contents/Resources/
fi

# Create Info.plist
cat > MetaImGUI.app/Contents/Info.plist << 'EOF'
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
    <key>CFBundleDisplayName</key>
    <string>MetaImGUI</string>
    <key>CFBundleVersion</key>
    <string>1.0.0</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleIconFile</key>
    <string>metaimgui.icns</string>
    <key>NSHighResolutionCapable</key>
    <true/>
</dict>
</plist>
EOF

# Test the app bundle
open MetaImGUI.app
```

**Verification checklist:**
- [ ] App bundle opens (may show security warning - see below)
- [ ] Application launches
- [ ] UI renders correctly
- [ ] Icon appears in Dock and Finder

#### Test 3: Create DMG

```bash
# Create DMG staging directory
mkdir -p dmg_contents
cp -r MetaImGUI.app dmg_contents/

# Add Applications symlink for better UX
ln -s /Applications dmg_contents/Applications

# Create DMG
hdiutil create -volname "MetaImGUI" -srcfolder dmg_contents -ov -format UDZO MetaImGUI-1.0.0-macos-x64.dmg

# Test the DMG
open MetaImGUI-1.0.0-macos-x64.dmg
```

**DMG verification:**
- [ ] DMG mounts successfully
- [ ] Shows MetaImGUI.app and Applications folder
- [ ] Can drag app to Applications folder
- [ ] App runs from Applications folder

#### Test 4: Code Signing (Optional)

If you have an Apple Developer certificate:

```bash
# Sign the executable
codesign --force --deep --sign "Developer ID Application: Your Name (TEAM_ID)" \
  --options runtime \
  --entitlements entitlements.plist \
  --timestamp \
  MetaImGUI.app/Contents/MacOS/MetaImGUI

# Sign the app bundle
codesign --force --deep --sign "Developer ID Application: Your Name (TEAM_ID)" \
  --options runtime \
  --timestamp \
  MetaImGUI.app

# Verify signature
codesign --verify --deep --strict --verbose=2 MetaImGUI.app
spctl -a -vv MetaImGUI.app
```

### Handling Gatekeeper (Unsigned Apps)

For unsigned apps, users will see "app is damaged" or "from unidentified developer":

**Bypass for testing:**
```bash
# Remove quarantine attribute
xattr -d com.apple.quarantine MetaImGUI.app

# Or allow in System Preferences
# System Preferences > Security & Privacy > General > "Open Anyway"
```

**For distribution:**
- Apps should be code-signed with a Developer ID
- Notarize the app with Apple
- See: https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution

### Troubleshooting macOS

**Issue: "MetaImGUI.app is damaged and can't be opened"**
- Cause: Gatekeeper blocking unsigned app
- Solution: `xattr -d com.apple.quarantine MetaImGUI.app`

**Issue: "Library not loaded: @rpath/libglfw.3.dylib"**
- Cause: GLFW not found
- Solution: `brew install glfw`

**Issue: "Icon not showing"**
- Cause: .icns file missing or not referenced in Info.plist
- Solution: Verify `CFBundleIconFile` in Info.plist and file exists

---

## Linux Testing

Already tested! See previous test results. To re-run:

```bash
./packaging/test_packages.sh 1.0.0
```

### Individual Format Testing

#### AppImage
```bash
# Build
./packaging/create_linux_packages.sh 1.0.0

# Run
chmod +x packaging/output/MetaImGUI-1.0.0-x86_64.AppImage
./packaging/output/MetaImGUI-1.0.0-x86_64.AppImage
```

#### .deb Package
```bash
# Install
sudo dpkg -i packaging/output/metaimgui_1.0.0_amd64.deb
sudo apt-get install -f  # Fix dependencies if needed

# Run
metaimgui

# Uninstall
sudo apt-get remove metaimgui
```

#### Flatpak
```bash
# Build and install
cd packaging/flatpak
flatpak-builder --user --install --force-clean build-dir com.metaimgui.MetaImGUI.yaml

# Run
flatpak run com.metaimgui.MetaImGUI

# Uninstall
flatpak uninstall com.metaimgui.MetaImGUI
```

#### Snap
```bash
# Build
cd packaging/snap
snapcraft

# Install
sudo snap install metaimgui_1.0.0_amd64.snap --dangerous

# Run
metaimgui

# Uninstall
sudo snap remove metaimgui
```

---

## Testing via GitHub Actions

To test the full release workflow:

1. **Create a test tag:**
   ```bash
   git tag v1.0.0-test
   git push origin v1.0.0-test
   ```

2. **Monitor the workflow:**
   - Go to: https://github.com/andynicholson/MetaImGUI/actions
   - Watch the "Release" workflow
   - It will build for all platforms: Linux, Windows, macOS

3. **Download and test artifacts:**
   - Go to: https://github.com/andynicholson/MetaImGUI/releases
   - Download packages for each platform:
     - **Linux**: `.tar.gz` and `.AppImage` (via `create_linux_packages.sh`)
     - **Windows**: `-Setup.exe` (installer) and `-portable.zip` (via `create_windows_installer.ps1`)
     - **macOS**: `.dmg` (automated in workflow)
   - Test on respective systems

4. **Clean up test release:**
   ```bash
   # Delete the tag
   git tag -d v1.0.0-test
   git push origin :refs/tags/v1.0.0-test

   # Delete the release on GitHub (via web interface)
   ```

**Note**: Windows and Linux now use the **same packaging scripts** locally and in CI/CD!

---

## Complete Test Matrix

### Windows

| Test | Status | Notes |
|------|--------|-------|
| Build with vcpkg | ⬜ | Verify no build errors |
| Portable ZIP extracts | ⬜ | All DLLs present |
| Portable runs | ⬜ | No DLL errors |
| NSIS installer runs | ⬜ | Correct version shown |
| Start Menu shortcuts | ⬜ | Created correctly |
| Desktop shortcut | ⬜ | Works if selected |
| Add/Remove Programs | ⬜ | Entry created |
| Application runs | ⬜ | All features work |
| Uninstaller works | ⬜ | Clean removal |

### macOS

| Test | Status | Notes |
|------|--------|-------|
| Build succeeds | ⬜ | No build errors |
| App bundle creates | ⬜ | Correct structure |
| App launches | ⬜ | May need quarantine fix |
| Icon shows | ⬜ | In Finder/Dock |
| DMG mounts | ⬜ | Shows app + Applications |
| Install to /Applications | ⬜ | Drag and drop works |
| Runs from Applications | ⬜ | All features work |

### Linux

| Test | Status | Notes |
|------|--------|-------|
| AppImage builds | ✅ | PASS |
| AppImage runs | ⬜ | Portable |
| .deb installs | ✅ | PASS |
| .deb runs | ⬜ | Installed via dpkg |
| Flatpak builds | ✅ | PASS |
| Flatpak runs | ⬜ | Sandboxed |
| Snap builds | ✅ | PASS |
| Snap runs | ⬜ | Confined |

---

## Quick Test Commands

### Windows (PowerShell)
```powershell
# One-line test
.\packaging\create_windows_installer.ps1 -Version 1.0.0; .\packaging\output\MetaImGUI-1.0.0-Setup.exe
```

### macOS (Bash)
```bash
# One-line test
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build && ./build/MetaImGUI
```

### Linux (Bash)
```bash
# One-line test all formats
./packaging/test_packages.sh 1.0.0
```

---

## Reporting Issues

When reporting packaging issues, include:

1. **Platform and version** (e.g., Windows 11, macOS 14 Sonoma, Ubuntu 24.04)
2. **Packaging format** (e.g., NSIS installer, DMG, .deb)
3. **Error messages** (full text or screenshots)
4. **Steps to reproduce**
5. **Expected vs actual behavior**

Example:
```
Platform: Windows 11 Pro
Format: NSIS Installer v1.0.0
Issue: Application fails to start with "libcurl.dll missing"
Steps: Installed via MetaImGUI-1.0.0-Setup.exe, launched from Start Menu
Expected: App starts normally
Actual: Error dialog about missing DLL
```

---

## Notes

- **Windows**: VC++ Runtime is required but not bundled. Users may need to install it.
- **macOS**: Unsigned apps will trigger Gatekeeper warnings. Code signing recommended for distribution.
- **Linux**: All formats tested and working on Ubuntu. May need testing on other distros.
- **GitHub Actions**: All platforms build automatically on tag push (v*.*.*).

