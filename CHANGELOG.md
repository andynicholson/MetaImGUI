# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

### Changed

### Deprecated

### Removed

### Fixed

### Security

## [1.3.0] - 2025-11-19

### Added
- Exit confirmation dialog now appears when clicking window close button (X)
- Localization support for About and demo window buttons
- Window close callback system in WindowManager
- Translation keys: `button.show_about`, `button.show_demo`, `button.close`
- CONTRIBUTING.md with contribution guidelines
- CODE_OF_CONDUCT.md (Contributor Covenant v2.1)
- SECURITY.md with vulnerability reporting process
- CHANGELOG.md (this file) for tracking changes


### Changed
- All main window and About dialog buttons now use localization system
- Exit confirmation dialog now intercepts all exit methods (menu, ESC key, close button)
- Improved documentation structure
- Enhanced project professionalism

### Fixed
- About dialog button and demo window button not translating when language changed
- Window close button (X) now properly shows confirmation dialog instead of closing immediately

## [1.2.0] - Initial Release

### Added
- Initial project template with ImGui v1.92.4 integration
- Cross-platform build system (CMake 3.16+)
- C++20 support with modern features
- Window management system (GLFW + OpenGL 3.3)
- Clean architecture with separated concerns:
  - Application class for lifecycle management
  - WindowManager for window operations
  - UIRenderer for UI rendering logic
  - ThemeManager for styling (4 built-in themes)
  - UpdateChecker for release notifications
- Complete CI/CD infrastructure:
  - GitHub Actions workflows for CI and releases
  - Multi-platform builds (Linux, Windows, macOS)
  - Automated packaging (AppImage, .deb, .zip, .dmg)
  - Code quality checks (clang-format, clang-tidy)
- Testing framework with Catch2 v3.4.0:
  - Unit tests for core components
  - CTest integration
  - Example test cases
- Template initialization system:
  - Interactive shell script (init_template.sh)
  - PowerShell script for Windows (init_template.ps1)
  - Automatic project renaming and customization
- Professional documentation:
  - Comprehensive README.md
  - META_FEATURES.md explaining meta-project features
  - QUICK_REFERENCE.md for common operations
  - SETUP_GUIDE.md for getting started
- Development tools:
  - VS Code integration with launch configurations
  - .clang-format for code formatting
  - .editorconfig for editor consistency
  - Git-based version management
- Dependency management:
  - Automated setup scripts
  - ImGui v1.92.4
  - nlohmann/json v3.11.3
  - Catch2 v3.4.0
- Update notification system:
  - Asynchronous update checking
  - GitHub Releases API integration
  - Non-blocking startup
- Packaging infrastructure:
  - Linux: AppImage, .deb, .tar.gz
  - Windows: Portable ZIP, NSIS installer
  - macOS: DMG with app bundle
  - SHA256 checksums for all releases
- Build scripts for quick setup:
  - build.sh for Linux/macOS
  - build.bat for Windows
  - setup_dependencies.sh/bat

### Technical Details
- C++20 standard throughout
- Modern C++ features: jthread, stop_token, designated initializers
- RAII and smart pointer usage
- Exception-safe resource management
- Thread-safe update checking
- Cross-platform compatibility
- Compiler support: GCC 10+, Clang 10+, MSVC 2019 16.11+

### Known Issues
- macOS code signing uses placeholder certificates
- Some compiler warnings on specific platforms (being addressed)

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
2. Creating and pushing a version tag (e.g., `v1.2.3`)
3. GitHub Actions automatically builds and creates the release
4. Assets are uploaded with SHA256 checksums

### Upgrading

When upgrading between versions:
1. Review this CHANGELOG for breaking changes
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

[Unreleased]: https://github.com/andynicholson/MetaImGUI/compare/v1.3.0...HEAD
[1.3.0]: https://github.com/andynicholson/MetaImGUI/compare/v1.2.0...v1.3.0
