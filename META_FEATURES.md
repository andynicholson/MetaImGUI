# MetaImGUI Meta-Features Documentation

Meta-project features included in the template.

## Overview

MetaImGUI provides infrastructure for ImGui applications: build system, CI/CD, testing, and packaging.

---

## 📦 Meta-Features

### 1. **CI/CD and Automated Releases**

**Location:** `.github/workflows/`

**What it does:**
- **CI Workflow** (`ci.yml`): Builds on Linux, Windows, macOS for each push/PR
- **Release Workflow** (`release.yml`): Creates releases with installers on tag push

**Usage:**
```bash
git tag v1.0.0
git push origin v1.0.0
```

GitHub Actions builds for all platforms, creates installers (AppImage, .deb, NSIS, DMG), generates changelog, publishes release.

---

### 2. **Automated Version Management**

**Location:** `cmake/GetGitVersion.cmake`, `include/version.h.in`

**What it does:**
Extracts version from git tags, embeds in executable.

**Usage:**
```bash
git tag v2.5.3
cmake -B build && cmake --build build
```

Version appears in executable metadata, about dialog, and CMake project version. Tracks git commit hash, branch, and build configuration.

---

### 3. **Update Notification System**

**Location:** `include/UpdateChecker.h`, `src/UpdateChecker.cpp`

**What it does:**
Checks GitHub releases API on startup. Shows notification if update available. Links to release page.

**How it works:**
Asynchronous GitHub API check, version comparison, notification dialog. Manual check available via Help menu. Uses libcurl on Linux/macOS, WinINet on Windows.

---

### 4. **Template Initialization System**

**Location:** `init_template.sh`, `init_template.ps1`

**What it does:**
Renames project, updates namespaces, modifies documentation.

**Usage:**
```bash
./init_template.sh  # Interactive

./init_template.sh --name "MyApp" --namespace "MyApp"  # Non-interactive

.\init_template.ps1 -Name "MyApp"  # Windows
```

Updates CMakeLists.txt, source files, namespaces, window titles, README, build scripts.

---

### 5. **Testing Framework**

**Location:** `tests/`

**What it does:**
Catch2 testing framework with sample tests. CMake/CTest integration.

**Usage:**
```bash
cmake -B build -DBUILD_TESTS=ON
cmake --build build
ctest --output-on-failure
```

Uses Catch2 v3. Includes tests for version checking, update logic, theme manager, config manager, logger, window manager.

---

### 6. **Code Quality Tools**

**Location:** `.clang-format`, `.clang-tidy`, `.editorconfig`

**What it does:**
Code formatting and static analysis configuration.

**Usage:**
```bash
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

Includes:
- `.clang-format`: C++ formatting rules
- `.clang-tidy`: Static analysis config
- `.editorconfig`: Editor settings

---

### 7. **Packaging Scripts**

**Location:** `packaging/`

**What it does:**
Creates installers for each platform.

**Linux:**
```bash
./packaging/create_linux_packages.sh 1.0.0
# Creates AppImage, .deb, .tar.gz
```

**Windows:**
```powershell
.\packaging\create_windows_installer.ps1 -Version 1.0.0
# Creates NSIS installer, portable ZIP
```

**macOS:**
```bash
./packaging/create_macos_dmg.sh 1.0.0
# Creates DMG installer
```

Includes desktop shortcuts, start menu entries, uninstallers.

---

### 8. **VS Code Development Environment**

**Location:** `.vscode/`

**What it does:**
VS Code workspace configuration.

**Includes:**
- `settings.json`: Editor settings
- `launch.json`: Debug configs (GDB, LLDB, MSVC)
- `tasks.json`: Build tasks
- `c_cpp_properties.json`: IntelliSense paths
- `extensions.json`: Recommended extensions

F5 to debug, Ctrl+Shift+B to build.

---

## 🚀 Quick Start

1. Clone repository
2. Run `./init_template.sh`
3. Run `./setup_dependencies.sh`
4. Run `./build.sh`
5. Tag and push: `git tag v1.0.0 && git push origin v1.0.0`

---

## 📋 Production Checklist

- [ ] Run `init_template.sh`
- [ ] Update `README.md`
- [ ] Add `LICENSE` file
- [ ] Replace icons in `packaging/`
- [ ] Update `UpdateChecker` with your GitHub repo
- [ ] Enable GitHub Actions
- [ ] Create git tag: `git tag v1.0.0`
- [ ] Update author info in packaging scripts

---

## 🔧 Customization Guide

### Changing Project Name

Use `init_template.sh` or manually edit:
- `CMakeLists.txt`
- Source files (namespace)
- `Application.h` (WINDOW_TITLE)
- Build scripts
- Packaging scripts

### Adding Features

- New source files: Add to `CMakeLists.txt`
- New dependencies: Update `setup_dependencies.sh`
- New UI: Add methods to `Application` class
- New tests: Create files in `tests/`

### Disabling Features

```cmake
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
```

Remove unwanted directories (`.github/`, `tests/`, `packaging/`, `.vscode/`).

---

## 📚 Additional Resources

### GitHub Actions
- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Creating Releases](https://docs.github.com/en/repositories/releasing-projects-on-github)

### CMake
- [CMake Documentation](https://cmake.org/documentation/)
- [Modern CMake](https://cliutils.gitlab.io/modern-cmake/)

### Testing
- [Catch2 Documentation](https://github.com/catchorg/Catch2/tree/devel/docs)

### Packaging
- [NSIS Documentation](https://nsis.sourceforge.io/Docs/)
- [AppImage Documentation](https://docs.appimage.org/)
- [Debian Packaging](https://www.debian.org/doc/manuals/maint-guide/)

---

## 🤝 Contributing to MetaImGUI

If you find issues or have improvements for the meta-features:

1. Open an issue describing the problem/enhancement
2. Submit a pull request with your changes
3. Update this documentation if needed

---

## 📄 License

The meta-features and template infrastructure are provided as-is for use in your projects. Customize and adapt as needed!

---

## What's Included

- CI/CD workflows
- Automated releases
- Update system
- Git-based versioning
- Testing framework
- Code quality tools
- Packaging scripts
- IDE configuration
- Init scripts

---

**Happy coding! 🚀**

