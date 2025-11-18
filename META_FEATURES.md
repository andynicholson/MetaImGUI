# MetaImGUI Meta-Features Documentation

This document describes all the meta-project features that make MetaImGUI an excellent template for starting new ImGui-based applications.

## 🎯 Overview

MetaImGUI is designed as a **meta-project** - a professional template that includes all the infrastructure needed to quickly bootstrap production-ready ImGui applications. This goes beyond a simple "Hello World" example by including industry-standard tooling, automation, and best practices.

---

## 📦 Meta-Features Included

### 1. **CI/CD and Automated Releases** ✅

**Location:** `.github/workflows/`

**What it does:**
- **CI Workflow** (`ci.yml`): Automatically builds your project on Linux, Windows, and macOS for every push and pull request
- **Release Workflow** (`release.yml`): Automatically creates releases with platform-specific installers when you push a version tag

**How to use:**
```bash
# Create a release
git tag v1.0.0
git push origin v1.0.0

# GitHub Actions will automatically:
# - Build for all platforms
# - Create installers (AppImage, .deb, .zip, .dmg)
# - Generate changelog from commits
# - Publish a GitHub Release
```

**Features:**
- Multi-platform builds (Linux, Windows, macOS)
- Automated packaging (AppImage, .deb, .zip, .dmg)
- Changelog generation from git commits
- Artifact caching for faster builds
- Code quality checks (clang-format)

---

### 2. **Automated Version Management** ✅

**Location:** `cmake/GetGitVersion.cmake`, `include/version.h.in`

**What it does:**
- Extracts version information directly from git tags
- Embeds version in the compiled executable
- No more manual version updates in multiple files!

**How to use:**
```bash
# Set version with git tag
git tag v2.5.3

# Build the project
cmake -B build && cmake --build build

# Version is automatically embedded in:
# - Executable metadata
# - About dialog
# - CMake project version
```

**Features:**
- Single source of truth (git tags)
- Automatic version parsing (major.minor.patch)
- Git commit hash and branch tracking
- Build configuration tracking

---

### 3. **Update Notification System** ✅

**Location:** `include/UpdateChecker.h`, `src/UpdateChecker.cpp`

**What it does:**
- Checks GitHub releases API for new versions on startup
- Shows a beautiful notification dialog when updates are available
- Non-blocking, asynchronous checking
- Direct links to download the latest release

**How it works:**
1. On app startup, checks GitHub API asynchronously
2. Compares current version with latest release
3. Shows notification if newer version exists
4. User can open release page with one click

**Features:**
- Non-blocking startup (happens in background)
- Works on Windows (WinINet) and Linux/macOS (curl)
- Manual check via Help menu
- Release notes preview
- Smart version comparison

---

### 4. **Template Initialization System** ✅

**Location:** `init_template.sh`, `init_template.ps1`

**What it does:**
- Interactive script to customize the template for your project
- Renames all occurrences of "MetaImGUI" to your project name
- Updates namespaces, window titles, and documentation
- Optionally creates a fresh git commit

**How to use:**
```bash
# Interactive mode
./init_template.sh

# Non-interactive mode
./init_template.sh \
  --name "MyAwesomeApp" \
  --namespace "MyApp" \
  --author "John Doe" \
  --description "My cool application" \
  --github-user "johndoe"

# Windows
.\init_template.ps1 -Name "MyAwesomeApp" -Namespace "MyApp"
```

**What it updates:**
- CMakeLists.txt (project name, executable name)
- All source files (namespace, class names)
- Window titles and UI strings
- README.md
- Build scripts
- Monitoring scripts

---

### 5. **Testing Framework** ✅

**Location:** `tests/`, uses Catch2

**What it does:**
- Integrated Catch2 testing framework
- Sample tests for version checking and update logic
- CMake integration with CTest
- Automatic test discovery

**How to use:**
```bash
# Build with tests
cmake -B build -DBUILD_TESTS=ON
cmake --build build

# Run tests
cd build
ctest --output-on-failure

# Or run the test executable directly
./build/MetaImGUI_tests
```

**Features:**
- Modern Catch2 v3 framework
- BDD-style test syntax
- Automatic test registration
- Parallel test execution
- Detailed failure reporting

---

### 6. **Code Quality Tools** ✅

**Location:** `.clang-format`, `.clang-tidy`, `.editorconfig`, `.pre-commit-hook.sh`

**What it does:**
- Enforces consistent code style across the project
- Provides static analysis with clang-tidy
- Works with any editor via EditorConfig
- Optional pre-commit hook for automatic formatting

**How to use:**
```bash
# Format all code
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Enable pre-commit hook
cp .pre-commit-hook.sh .git/hooks/pre-commit

# VS Code will auto-format on save (if configured)
```

**Includes:**
- `.clang-format`: LLVM-based C++ formatting rules
- `.clang-tidy`: Static analysis configuration
- `.editorconfig`: Universal editor settings
- Pre-commit hook: Automatic formatting before commits

---

### 7. **Professional Packaging Scripts** ✅

**Location:** `packaging/`

**What it does:**
- Creates production-ready installers for each platform
- Bundles all dependencies
- Generates platform-specific packages

**Linux** (`packaging/create_linux_packages.sh`):
```bash
./packaging/create_linux_packages.sh 1.0.0

# Creates:
# - AppImage (portable, runs anywhere)
# - .deb package (Ubuntu/Debian)
# - .tar.gz (generic Linux)
```

**Windows** (`packaging/create_windows_installer.ps1`):
```powershell
.\packaging\create_windows_installer.ps1 -Version 1.0.0

# Creates:
# - NSIS installer (professional setup.exe)
# - Portable ZIP (no installation needed)
```

**Features:**
- Automatic dependency bundling
- Desktop shortcuts
- Start menu entries
- Uninstallers
- Professional metadata

---

### 8. **VS Code Development Environment** ✅

**Location:** `.vscode/`

**What it does:**
- Pre-configured Visual Studio Code workspace
- One-click building and debugging
- IntelliSense configuration
- Recommended extensions

**Includes:**
- `settings.json`: Editor settings, C++ configuration
- `launch.json`: Debug configurations (GDB, LLDB, MSVC)
- `tasks.json`: Build tasks (CMake configure, build, clean)
- `c_cpp_properties.json`: IntelliSense paths
- `extensions.json`: Recommended extensions

**Features:**
- F5 to build and debug
- Ctrl+Shift+B to build
- Automatic formatting on save
- Integrated CMake Tools
- Multi-platform debug configs

---

## 🚀 Quick Start for Template Users

### As a New Project

1. **Clone or download MetaImGUI**
   ```bash
   git clone https://github.com/yourusername/MetaImGUI.git MyProject
   cd MyProject
   ```

2. **Initialize your project**
   ```bash
   ./init_template.sh
   # Follow the prompts to customize
   ```

3. **Set up dependencies**
   ```bash
   ./setup_dependencies.sh
   ```

4. **Build**
   ```bash
   ./build.sh
   ```

5. **Create your first release**
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   # GitHub Actions will build everything!
   ```

---

## 📋 Checklist for Production Use

After initializing from the template:

- [ ] Run `init_template.sh` to customize project names
- [ ] Update `README.md` with your project details
- [ ] Add a `LICENSE` file
- [ ] Replace placeholder icon in `packaging/`
- [ ] Update `UpdateChecker` initialization with your GitHub repo:
  ```cpp
  m_updateChecker = std::make_unique<UpdateChecker>("your-username", "your-repo");
  ```
- [ ] Configure GitHub repository:
  - [ ] Enable GitHub Actions
  - [ ] Set repository secrets if needed
  - [ ] Enable GitHub Releases
- [ ] Create initial git tag: `git tag v1.0.0`
- [ ] Update author information in:
  - [ ] `packaging/windows_installer.nsi`
  - [ ] `packaging/create_linux_packages.sh`
  - [ ] Source file headers (optional)

---

## 🔧 Customization Guide

### Changing the Project Name

Use the initialization script, or manually:

1. **CMakeLists.txt**: Update `project(YourProjectName ...)`
2. **Source files**: Replace `MetaImGUI` namespace
3. **Application.h**: Update `WINDOW_TITLE`
4. **Build scripts**: Update project name references
5. **Packaging scripts**: Update `PROJECT_NAME` variables

### Adding New Features

1. **New source files**: Add to `CMakeLists.txt`
2. **New dependencies**: Update `setup_dependencies.sh`
3. **New UI windows**: Add render methods to `Application` class
4. **New tests**: Create files in `tests/` directory

### Disabling Features

In `CMakeLists.txt`:
```cmake
option(BUILD_TESTS "Build tests" OFF)  # Disable tests
```

Remove directories you don't need:
- `.github/` - No CI/CD
- `tests/` - No testing
- `packaging/` - No automated packaging
- `.vscode/` - No VS Code integration

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

## 🎉 What Makes This a Meta-Project?

Unlike typical "starter templates," MetaImGUI includes:

✅ **Production-Ready CI/CD** - Not just build scripts, but full automation
✅ **Automated Releases** - Push a tag, get installers for all platforms
✅ **Built-in Update System** - Users stay current automatically
✅ **Version Management** - No manual version tracking needed
✅ **Testing Framework** - Start with tests from day one
✅ **Code Quality Tools** - Maintain consistency as you grow
✅ **Professional Packaging** - Ship real installers, not just executables
✅ **IDE Integration** - Start coding immediately in VS Code
✅ **Template Initialization** - One script to make it yours

This is everything you need to **ship a real product**, not just a prototype.

---

**Happy coding! 🚀**

