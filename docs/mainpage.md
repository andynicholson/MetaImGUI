# MetaImGUI API Documentation {#mainpage}

Welcome to the MetaImGUI API documentation. MetaImGUI is a professional C++20 template for creating ImGui-based desktop applications with complete CI/CD infrastructure.

## Overview

MetaImGUI provides a solid foundation for building cross-platform desktop applications using:
- **ImGui v1.92.4** - Immediate mode GUI library
- **GLFW 3.x** - Cross-platform window management
- **OpenGL 3.3** - Graphics API
- **C++20** - Modern C++ with latest features

## Key Features

### Core Components

- @ref MetaImGUI::Application "Application" - Main application orchestration
- @ref MetaImGUI::WindowManager "WindowManager" - GLFW window management
- @ref MetaImGUI::UIRenderer "UIRenderer" - ImGui rendering and layout
- @ref MetaImGUI::ConfigManager "ConfigManager" - JSON configuration persistence
- @ref MetaImGUI::Logger "Logger" - Thread-safe logging system
- @ref MetaImGUI::ThemeManager "ThemeManager" - UI theming and styling
- @ref MetaImGUI::DialogManager "DialogManager" - Reusable dialog components
- @ref MetaImGUI::Localization "Localization" - Multi-language support
- @ref MetaImGUI::UpdateChecker "UpdateChecker" - Asynchronous update notifications

### Architecture

The codebase follows SOLID principles with clear separation of concerns:

```
┌─────────────────┐
│   Application   │  ← Main orchestration
├─────────────────┤
│ WindowManager   │  ← GLFW + OpenGL
│  UIRenderer     │  ← ImGui rendering
│ ConfigManager   │  ← Settings persistence
│  Logger         │  ← Logging system
│ DialogManager   │  ← UI dialogs
│ ThemeManager    │  ← Styling
│ Localization    │  ← i18n support
│ UpdateChecker   │  ← Update notifications
└─────────────────┘
```

## Quick Start

### Basic Usage

```cpp
#include "Application.h"

int main() {
    MetaImGUI::Application app;

    if (!app.Initialize()) {
        return 1;
    }

    app.Run();
    app.Shutdown();

    return 0;
}
```

### Logging

```cpp
#include "Logger.h"

// Initialize logger
Logger::Instance().Initialize("logs/app.log", LogLevel::Info);

// Use logging macros
LOG_INFO("Application started");
LOG_ERROR("Failed to load file: {}", filename);
LOG_DEBUG("Debug value: {}", value);
```

### Configuration

```cpp
#include "ConfigManager.h"

ConfigManager config;
config.Load();

// Get/set values
auto windowSize = config.GetWindowSize();
config.SetString("theme", "dark");

config.Save();
```

### Localization

```cpp
#include "Localization.h"

// Load translations
Localization::Instance().LoadTranslations("resources/translations/translations.json");

// Set language
Localization::Instance().SetLanguage("es");  // Spanish

// Get translated string
std::string greeting = Localization::Instance().Get("menu.file");
```

## Building

### Prerequisites

- CMake 3.16+
- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019 16.11+)
- GLFW 3.x
- OpenGL 3.3+
- libcurl (for update checking)

### Build Commands

```bash
# Setup dependencies
./setup_dependencies.sh

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Run
./build/MetaImGUI
```

### CMake Options

- `BUILD_TESTS` - Build test suite (default: ON)
- `ENABLE_COVERAGE` - Enable code coverage (default: OFF)
- `CMAKE_BUILD_TYPE` - Build type: Debug, Release, RelWithDebInfo, MinSizeRel

## Testing

```bash
# Build with tests
cmake -B build -DBUILD_TESTS=ON
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure
```

## Code Coverage

```bash
# Quick coverage report
./scripts/run_coverage.sh

# Manual
cmake -B build -DENABLE_COVERAGE=ON -DBUILD_TESTS=ON
cmake --build build
cd build && make coverage
```

## Platform Support

- **Linux** - Ubuntu 20.04+, Fedora 33+, Arch, Debian
- **Windows** - Windows 10/11 with MSVC 2019+
- **macOS** - macOS 11+ with Apple Clang

## Additional Resources

- [GitHub Repository](https://github.com/andynicholson/MetaImGUI)
- [README](https://github.com/andynicholson/MetaImGUI/blob/main/README.md)
- [Contributing Guide](https://github.com/andynicholson/MetaImGUI/blob/main/CONTRIBUTING.md)
- [Quick Reference](https://github.com/andynicholson/MetaImGUI/blob/main/QUICK_REFERENCE.md)

## License

GNU General Public License v3.0. See [LICENSE](https://github.com/andynicholson/MetaImGUI/blob/main/LICENSE) for details.

