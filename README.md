# MetaImGUI

[![CI Build](https://github.com/andynicholson/MetaImGUI/actions/workflows/ci.yml/badge.svg)](https://github.com/andynicholson/MetaImGUI/actions/workflows/ci.yml)
[![Release Build](https://github.com/andynicholson/MetaImGUI/actions/workflows/release.yml/badge.svg)](https://github.com/andynicholson/MetaImGUI/actions/workflows/release.yml)
[![Latest Release](https://img.shields.io/github/v/release/andynicholson/MetaImGUI)](https://github.com/andynicholson/MetaImGUI/releases/latest)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey)](https://github.com/andynicholson/MetaImGUI)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![ImGui](https://img.shields.io/badge/ImGui-1.92.4-orange)](https://github.com/ocornut/imgui)

A template project for creating ImGui-based desktop applications with a modern C++20 codebase and cross-platform build system. Designed for efficient development in AI-assisted editors like Cursor.

## Overview

MetaImGUI is a minimal template that provides the essential structure for building ImGui-based desktop applications. It includes complete build infrastructure, dependency management, and CI/CD automation. The project is structured to work well with AI-assisted development tools, providing clear patterns and organization that AI assistants can understand and extend.

## Features

### Core Application
- 🚀 **Modern C++20** codebase with proper error handling
- 🔧 **Cross-platform build system** using CMake
- 🎨 **ImGui v1.92.4** integration with GLFW and OpenGL 3.3
- 🏗️ **Clean architecture** with separated concerns
- 🖼️ **Basic UI structure** with menu bar and about dialog
- ⚙️ **Settings persistence** - JSON-based configuration management (window size, preferences)
- 📝 **Logging system** - Thread-safe multi-level logging (console + file output)
- 💬 **Dialog system** - Reusable dialogs (message boxes, input, progress, lists)
- 🌍 **Localization** - Multi-language support (English, Spanish, French, German)

### Infrastructure & Tooling
- ⚡ **Automated CI/CD** - Cross-platform builds (Linux, Windows, macOS) on every commit
- 📦 **Automated releases** - Tag-based release generation with platform-specific installers
- 🔔 **Update notifications** - Built-in update checker for end users
- 🏷️ **Git-based versioning** - Version numbers extracted from git tags
- 🎯 **Template initialization** - Script to customize project name and namespace
- ✅ **Testing framework** - Catch2 integration with sample tests
- 🔍 **Code quality tools** - clang-format, clang-tidy, and EditorConfig configured
- 📋 **Professional packaging** - AppImage, .deb, NSIS installer, and DMG support
- 🤖 **Editor integration** - Pre-configured for VS Code and AI-assisted editors

### Designed for AI-Assisted Development
The codebase follows clear patterns and conventions that work well with AI coding assistants:
- 📐 Consistent naming conventions and project structure
- 📖 Well-documented code with clear separation of concerns
- 🎓 Standard C++ patterns that AI models recognize
- 🛠️ Complete build system that AI can modify confidently
- 📁 Straightforward file organization

> **See [META_FEATURES.md](META_FEATURES.md) for detailed documentation of all features.**

## Project Structure

```
MetaImGUI/
├── CMakeLists.txt              # Main build configuration
├── README.md                   # This file
├── LICENSE                     # GPL v3.0 license
├── AUTHORS                     # Project contributors
├── META_FEATURES.md            # Detailed feature documentation
├── QUICK_REFERENCE.md          # Quick reference guide
├── SETUP_GUIDE.md              # Setup instructions
│
├── src/                        # Source files
│   ├── main.cpp               # Application entry point
│   ├── Application.cpp        # Main application logic
│   ├── WindowManager.cpp      # Window management
│   ├── ThemeManager.cpp       # Theme/styling system
│   ├── UIRenderer.cpp         # UI rendering logic
│   ├── UpdateChecker.cpp      # Update notification system
│   ├── ConfigManager.cpp      # Settings persistence
│   ├── Logger.cpp             # Logging system
│   ├── DialogManager.cpp      # Dialog system
│   └── Localization.cpp       # Localization/translations
│
├── include/                    # Header files
│   ├── Application.h          # Application header
│   ├── WindowManager.h        # Window manager header
│   ├── ThemeManager.h         # Theme manager header
│   ├── UIRenderer.h           # UI renderer header
│   ├── UpdateChecker.h        # Update checker header
│   ├── ConfigManager.h        # Config manager header
│   ├── Logger.h               # Logger header
│   ├── DialogManager.h        # Dialog manager header
│   ├── Localization.h         # Localization header
│   └── version.h.in           # Version template
│
├── tests/                      # Test suite (Catch2)
│   ├── test_main.cpp          # Test runner
│   ├── test_theme_manager.cpp # Theme tests
│   ├── test_update_checker.cpp# Update checker tests
│   ├── test_version.cpp       # Version tests
│   ├── test_config_manager.cpp# Config manager tests
│   ├── test_logger.cpp        # Logger tests
│   └── test_window_manager.cpp# Window manager tests
│
├── cmake/                      # CMake modules
│   └── GetGitVersion.cmake    # Git version extraction
│
├── scripts/                    # Utility scripts
│   ├── monitor_actions.sh     # Linux/macOS workflow monitor
│   ├── monitor_actions.ps1    # Windows workflow monitor
│   ├── quick_status.sh        # Quick status check
│   └── README.md              # Scripts documentation
│
├── packaging/                  # Platform-specific packaging
│   ├── create_linux_packages.sh   # Linux package creation
│   ├── create_windows_installer.ps1 # Windows installer
│   └── windows_installer.nsi  # NSIS installer script
│
├── .github/workflows/          # GitHub Actions CI/CD
│   ├── ci.yml                 # Continuous Integration
│   └── release.yml            # Release automation
│
├── setup_dependencies.sh/.bat  # Dependency setup scripts
├── build.sh/.bat              # Build scripts
├── init_template.sh/.ps1      # Project initialization
├── resources/                 # Application resources
│   ├── translations/          # Translation files
│   │   └── translations.json # Multi-language strings
│   └── icons/                # Application icons
├── external/                  # External dependencies (generated)
│   ├── imgui/                # ImGui library
│   ├── json/                 # nlohmann/json
│   └── catch2/               # Catch2 testing framework
└── build/                     # Build output (generated)
```

## Quick Start

### Using as a Template for Your Project

The recommended workflow:

```bash
# 1. Fork this repository on GitHub (click the "Fork" button)

# 2. Clone your fork (replace YOUR_USERNAME with your GitHub username)
git clone https://github.com/YOUR_USERNAME/MetaImGUI.git MyProject
cd MyProject

# 3. Initialize your project (interactive)
./init_template.sh
# Follow the prompts to customize project name, namespace, etc.

# 4. Setup dependencies
./setup_dependencies.sh

# 5. Build
./build.sh

# 6. Run
./build/MyProject

# 7. Open in your AI-assisted editor (e.g., Cursor) and start developing
cursor .

# 8. When ready, create your first release
git tag v1.0.0
git push origin v1.0.0
# GitHub Actions will build installers for all platforms
```

### Prerequisites

**All Platforms:**
- 🔨 CMake 3.16 or higher
- ⚙️ C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019 16.11+)
- 🌿 Git (for downloading dependencies)

**Platform-Specific:**
- 🐧 **Linux**: `libcurl4-openssl-dev`, `libglfw3-dev`, `libgl1-mesa-dev`, `libglu1-mesa-dev`, `xorg-dev`
- 🪟 **Windows**: vcpkg (for GLFW and libcurl)
- 🍎 **macOS**: Homebrew (for GLFW and other dependencies)

### Building Without Initialization

If you just want to try the template as-is:

```bash
# Setup dependencies
chmod +x setup_dependencies.sh
./setup_dependencies.sh

# Build
chmod +x build.sh
./build.sh

# Run
./build/MetaImGUI
```

## Building Manually

If you prefer to build manually:

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)  # or make -j4 on Windows
```

## Customization

### Renaming the Project

**Recommended: Use the initialization script**
```bash
./init_template.sh  # Linux/macOS
# or
.\init_template.ps1  # Windows
```

The script automatically updates all project files, namespaces, and configurations.

**Manual customization** (if needed):
1. Update `CMakeLists.txt` - Change project name and executable references
2. Update all source files - Change namespace from `MetaImGUI` to your name
3. Update `include/` headers - Update WINDOW_TITLE and other constants
4. Update GitHub URLs in badges and documentation

### Architecture Overview

The template uses a modular architecture:

- **Application** - Main application lifecycle and coordination
- **WindowManager** - Window creation, event handling, and lifecycle
- **ThemeManager** - UI theming and styling (multiple built-in themes)
- **UIRenderer** - Core UI rendering logic and layout
- **UpdateChecker** - Automatic update notifications from GitHub releases
- **ConfigManager** - JSON-based settings persistence (window state, preferences)
- **Logger** - Thread-safe logging with file and console output
- **DialogManager** - Reusable UI dialogs (message boxes, input, progress, lists)
- **Localization** - Multi-language support with runtime language switching

### Adding New Features

The modular architecture makes it easy to extend:

- **New UI components** - Add rendering methods to `UIRenderer`
- **Additional themes** - Extend `ThemeManager` with new color schemes
- **Menu items** - Modify the menu bar in `UIRenderer::RenderMenuBar()`
- **Windows/dialogs** - Use `DialogManager` for message boxes, input, progress bars
- **Settings persistence** - Use `ConfigManager` to save/load configuration values
- **Logging** - Use `Logger::Instance()` with LOG_INFO, LOG_ERROR, LOG_DEBUG macros
- **Translations** - Add new languages to `resources/translations/translations.json`
- **Custom managers** - Create new manager classes following existing patterns

### Working with AI Assistants

When working in AI-assisted editors like Cursor:

1. **Ask for specific features** - "Add a settings window with theme selection"
2. **Request modifications** - "Update the menu bar to include a File menu"
3. **Code review** - "Review the Application class for potential improvements"
4. **Extend functionality** - "Add support for loading and displaying images"

The codebase structure allows AI assistants to understand the project layout and make coherent modifications across multiple files.

### GitHub Actions and Monitoring

The project includes GitHub Actions workflows for:

- **CI/CD workflows** - Automated builds and tests for Linux, Windows, and macOS
- **Release automation** - Package generation and release creation on git tags
- **Monitoring scripts** - Real-time workflow status tracking
- **Code quality checks** - Automated linting and formatting verification

For detailed information about the monitoring scripts, see `scripts/README.md`.

## Dependencies

### Core Dependencies
- 🎨 **ImGui v1.92.4**: Immediate mode GUI library
- 🪟 **GLFW 3.x**: Cross-platform window management
- 🎮 **OpenGL 3.3**: Graphics API
- 🌐 **libcurl**: HTTP client for update checking
- 📦 **nlohmann/json v3.11.3**: JSON parsing library

### Build & Development
- 🔨 **CMake 3.16+**: Build system generator
- ✅ **Catch2 v3.x**: Testing framework
- 🛠️ **C++20 Standard Library**: Modern C++ features (ranges, concepts, designated initializers)

## Platform Support

- 🐧 **Linux** - Tested on Ubuntu 20.04+, Fedora 33+
- 🪟 **Windows** - Tested on Windows 10/11 with MSVC
- 🍎 **macOS** - Supported (tested on macOS 11+)

All platforms are built and tested via GitHub Actions CI/CD pipeline.

## Troubleshooting

### Common Issues

1. **ImGui not found**: Run `./setup_dependencies.sh` first
2. **GLFW not found**: Install system dependencies via the setup script
3. **OpenGL errors**: Ensure your graphics drivers are up to date
4. **Build failures**: Check that you have a C++20 compatible compiler

### Debug Mode

Build with debug information:
```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Use Cases

This template is suitable for:

- 🔧 Desktop utilities and tools
- 📊 Data visualization applications
- ⚙️ Configuration and management interfaces
- 🎨 Prototyping GUI applications
- 🌍 Cross-platform desktop software

The template focuses on providing solid infrastructure so you can concentrate on building your application's unique features.

## Contributing

This is a template project. If you find issues or have improvements, feel free to open issues or pull requests.

## Acknowledgments

- [ImGui](https://github.com/ocornut/imgui) by Omar Cornut for the immediate mode GUI library
- [GLFW](https://www.glfw.org/) team for cross-platform window management
- [Catch2](https://github.com/catchorg/Catch2) for the modern C++ testing framework
- [nlohmann/json](https://github.com/nlohmann/json) by Niels Lohmann for JSON parsing
- [libcurl](https://curl.se/libcurl/) team for HTTP client functionality
