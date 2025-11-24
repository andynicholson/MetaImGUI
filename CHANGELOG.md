# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
## [1.0.4] - 2025-11-24

### Fixed
-  Fix macOS app crash by bundling dynamic libraries and setting correct deployment target

### Changed
-  release vy

## [y] - 2025-11-24

### Fixed
-  Fix macOS app crash by bundling dynamic libraries and setting correct deployment target

## [y] - 2025-11-24

### Fixed
-  Fix macOS app crash by bundling dynamic libraries and setting correct deployment target

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


### Added

### Changed

### Deprecated

### Removed

### Fixed

### Security

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
