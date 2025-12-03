# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
## [1.1.0] - 2025-12-03

### Added
-  add ISS Tracker with ImPlot visualization
-  add input dialog with localization and single source of truth for translations

### Fixed
-  remove continuous focus stealing from About window
-  fix main window stealing focus and inconsistent button behavior
-  add platform-specific math library linking for ImPlot
- UIRenderer): fix ISS Tracker window disappearing unexpectedly
- ISSTracker): fix stop token not being respected by thread

### Changed
-  enabled screenshots in README
-  enabled screenshots in README
-  enabled screenshots in README
-  Updates for correctness, screenshots
-  convert all remaining source files to use Logger
-  convert ISSTracker and UpdateChecker to use Logger

## [1.0.21] - 2025-11-25

### Changed
-  A large review for accuracy, tone and style

## [1.0.13] - 2025-11-25

### Fixed
-  Windows NSIS installer - use /nonfatal for optional DLLs

### Changed
- (deps): bump actions/upload-pages-artifact from 3 to 4
- (deps): bump actions/checkout from 4 to 6

## [1.0.12] - 2025-11-24

### Fixed
-  Windows installer - handle missing OpenSSL DLLs gracefully

## [1.0.11] - 2025-11-24

### Fixed
-  MacOS building fixes again
-  use LLVM with system C++ library for macOS compatibility
-  macOS 13.0 compatibility by using system compiler

## [1.0.10] - 2025-11-24

### Fixed
-  bundle all required DLLs for Windows installer and portable ZIP

## [1.0.9] - 2025-11-24

### Fixed
-  use ad-hoc signing for macOS when Developer ID certificate unavailable
-  add missing 'resources/' directory in translation paths for AppImage and system installs

## [1.0.8] - 2025-11-24

### Fixed
-  create Frameworks directory before bundling libraries on macOS

## [1.0.7] - 2025-11-24

### Fixed
-  correct path check for translations.json in AppImage verification
-  macOS release workflow to use CMake-built bundle

## [1.0.6] - 2025-11-24

### Fixed
-  macos packaging issue fixes
-  macos packaging issue fixes
-  macos packaging issue fixes

## [1.0.5] - 2025-11-24

### Fixed
-  fix packaging issues with translations

## [1.0.4] - 2025-11-24

### Fixed
-  Fix macOS app crash by bundling dynamic libraries and setting correct deployment target

## [1.0.3] - 2025-11-24

### Fixed
-  update changes to MacOS deployment target

## [1.0.2] - 2025-11-21

### Fixed
-  windows packaging

## [1.0.1] - 2025-11-21

### Fixed
-  packaging updates across Linux , Mac and Windows

### Changed
-  monitoring script fixes

---

## [1.0.0] - 2025-11-19

### Added
- Production-ready ImGui C++20 application template
- Cross-platform build system (Linux, Windows, macOS)
- Comprehensive CI/CD with GitHub Actions
- Automated code coverage tracking (Codecov)
- Memory safety checks (ASan, UBSan, TSan)
- Static analysis (clang-tidy, cppcheck)
- Security scanning (CodeQL)
- Performance benchmarks (Google Benchmark)
- Professional packaging (AppImage, .deb, NSIS, DMG)
- Complete documentation and guides
- Conventional Commits validation
- VS Code Dev Container support

---

## Version History

### Version Numbering

This project follows [Semantic Versioning](https://semver.org/):
- **MAJOR**: Incompatible API changes
- **MINOR**: Functionality added in a backwards compatible manner
- **PATCH**: Backwards compatible bug fixes

### Release Process

Releases are created by:
1. Updating this CHANGELOG.md
2. Creating and pushing a version tag (e.g., `v1.0.0`)
3. GitHub Actions automatically builds and creates the release
4. Assets are uploaded with SHA256 checksums

### Upgrading

When upgrading between versions:
1. Review this CHANGELOG.md for breaking changes
2. Update your fork/clone: `git pull upstream main`
3. Run setup script: `./setup_dependencies.sh`
4. Rebuild: `./build.sh`
5. Run tests: `cd build && ctest`

### Support Policy

- **Latest version**: Fully supported with updates and security fixes
- **Previous versions**: Bug fixes only (no new features)
- **Older versions**: Not supported, please upgrade

---

## Links

- [GitHub Repository](https://github.com/andynicholson/MetaImGUI)
- [Issue Tracker](https://github.com/andynicholson/MetaImGUI/issues)
- [Releases](https://github.com/andynicholson/MetaImGUI/releases)
- [Contributing Guide](CONTRIBUTING.md)
- [Code of Conduct](CODE_OF_CONDUCT.md)
