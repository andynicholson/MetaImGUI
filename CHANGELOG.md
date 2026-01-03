# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.0.0] - 2026-01-03

Initial production release of MetaImGUI.

### Added

**Core Application**
- C++20 ImGui application template with modern patterns
- Cross-platform support (Linux, Windows, macOS)
- OpenGL 4.6 rendering (4.1 on macOS with Core Profile)
- ImPlot integration for real-time plotting
- Thread-safe logging system with platform-specific paths
- JSON configuration management
- Localization system supporting 4 languages
- Comprehensive dialog system
- Automatic update checking

**Developer Experience**
- CMake build system with cross-platform presets
- Init scripts for project customization
- VS Code dev container configuration
- Comprehensive documentation suite

**Build & Quality Assurance**
- GitHub Actions CI/CD for all platforms
- Automated release builds with installers
- Static analysis with 400+ clang-tidy checks
- Memory safety testing (ASan, UBSan, TSan)
- Code coverage tracking
- Security scanning with CodeQL
- Performance benchmarking

**Packaging**
- Linux: AppImage, .deb, tarball
- Windows: NSIS installer, portable ZIP
- macOS: DMG with code signing support

**Documentation**
- Quick reference guide
- Setup guide
- GitHub Actions guide
- Security checklist
- Packaging guides
- Feature documentation

### Platform-Specific Features

**macOS**
- Proper app bundle resource resolution
- OpenGL Core Profile support
- Entitlements for security
- LLVM compiler with system C++ library
- Translations loading from bundle (Finder-safe)

**Windows**
- Static MSVC runtime linking
- NSIS installer with proper DLL bundling
- Portable ZIP distribution

**Linux**
- AppImage with custom AppRun
- System package installation
- Desktop integration

---

## Version History

This project follows [Semantic Versioning](https://semver.org/):
- **MAJOR**: Incompatible API changes
- **MINOR**: New features (backwards compatible)
- **PATCH**: Bug fixes (backwards compatible)

---

## Links

- [GitHub Repository](https://github.com/andynicholson/MetaImGUI)
- [Issue Tracker](https://github.com/andynicholson/MetaImGUI/issues)
- [Releases](https://github.com/andynicholson/MetaImGUI/releases)
- [Contributing Guide](CONTRIBUTING.md)
- [Code of Conduct](CODE_OF_CONDUCT.md)
