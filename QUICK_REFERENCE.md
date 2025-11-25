# MetaImGUI Quick Reference

Common commands and operations.

## 🚀 Quick Start

```bash
./init_template.sh
./setup_dependencies.sh
./build.sh
./build/YourProjectName
```

## 📋 Common Commands

### Building

```bash
# Debug build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Release build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Clean rebuild
rm -rf build && ./build.sh
```

### Testing

```bash
# Run all tests
ctest --test-dir build --output-on-failure

# Run specific test
./build/MetaImGUI_tests "UpdateChecker version comparison"

# Run with verbose output
./build/MetaImGUI_tests -v
```

### Code Coverage

```bash
./scripts/run_coverage.sh

# Manual
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -DBUILD_TESTS=ON
cmake --build build
cd build && make coverage
xdg-open build/coverage_html/index.html  # Linux
open build/coverage_html/index.html      # macOS
```

### Static Analysis

```bash
./scripts/run_static_analysis.sh

# With CMake
cmake -B build -DENABLE_CLANG_TIDY=ON -DENABLE_CPPCHECK=ON
cmake --build build

# Manual
clang-tidy -p build src/*.cpp include/*.h
cppcheck --enable=all src/ include/
```

### Sanitizers

```bash
./scripts/run_sanitizers.sh ALL
./scripts/run_sanitizers.sh ASAN
./scripts/run_sanitizers.sh TSAN
./scripts/run_sanitizers.sh UBSAN

# Manual
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON -DBUILD_TESTS=ON
cmake --build build
cd build && ASAN_OPTIONS="detect_leaks=1" ctest --output-on-failure
```

### Benchmarks

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON
cmake --build build
./build/benchmarks/MetaImGUI_benchmarks

# Specific benchmark
./build/benchmarks/MetaImGUI_benchmarks --benchmark_filter=Config

# JSON output
./build/benchmarks/MetaImGUI_benchmarks --benchmark_out=results.json
```

### Code Quality

```bash
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run --Werror
```

## 📦 Packaging

### Linux

```bash
# Create all Linux packages
./packaging/create_linux_packages.sh 1.0.0

# Output: AppImage, .deb, .tar.gz in packaging/output/
```

### Windows

```powershell
# Create Windows installer and portable ZIP
.\packaging\create_windows_installer.ps1 -Version 1.0.0

# Output: .exe installer and .zip in packaging\output\
```

## 🔄 Version Management

```bash
# View current version
git describe --tags

# Create new version
git tag v1.2.3
git push origin v1.2.3

# List all versions
git tag -l
```

## 🤖 CI/CD

### Trigger CI Build

```bash
# Push to main/develop
git push origin main

# Create pull request (triggers CI)
gh pr create
```

### Trigger Release

```bash
# Tag and push (triggers release workflow)
git tag v1.0.0
git push origin v1.0.0

# Check workflow status
gh run list
gh run watch
```

## 🛠️ Development

### VS Code

- **Build**: `Ctrl+Shift+B`
- **Debug**: `F5`
- **Format Document**: `Shift+Alt+F`
- **Format Selection**: `Ctrl+K Ctrl+F`

### CMake Options

```bash
# Disable tests
cmake -B build -DBUILD_TESTS=OFF

# Custom install prefix
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local

# Export compile commands (for clangd)
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

## 🔧 Configuration

### Update Checker

Edit `src/Application.cpp`:

```cpp
m_updateChecker = std::make_unique<UpdateChecker>(
    "your-github-username",
    "your-repo-name"
);
```

### Window Title

Edit `include/Application.h`:

```cpp
static constexpr const char* WINDOW_TITLE = "Your App Name";
```

### CMake Project Name

Edit `CMakeLists.txt`:

```cmake
project(YourProjectName VERSION ${PROJECT_VERSION} LANGUAGES CXX)
```

## 🐛 Troubleshooting

### Build Issues

```bash
# Clean build directory
rm -rf build

# Re-setup dependencies
./setup_dependencies.sh

# Rebuild
./build.sh
```

### Missing Dependencies

```bash
# Linux (Ubuntu/Debian)
sudo apt-get install libglfw3-dev libgl1-mesa-dev libcurl4-openssl-dev

# macOS
brew install cmake glfw

# Windows
vcpkg install glfw3:x64-windows
```

### Git Issues

```bash
# Reset to last commit
git reset --hard HEAD

# Clean untracked files
git clean -fd

# View status
git status
```

## 📚 File Locations

| What | Where |
|------|-------|
| Source code | `src/`, `include/` |
| Tests | `tests/` |
| Build output | `build/` |
| Packages | `packaging/output/` |
| CI/CD | `.github/workflows/` |
| VS Code config | `.vscode/` |
| Documentation | `*.md` files |

## 🎯 Release Checklist

- [ ] Update version in code if needed
- [ ] Run tests: `ctest --test-dir build`
- [ ] Format code: `clang-format -i src/* include/*`
- [ ] Update CHANGELOG.md (optional)
- [ ] Commit changes
- [ ] Tag version: `git tag v1.0.0`
- [ ] Push tag: `git push origin v1.0.0`
- [ ] Wait for CI/CD to complete
- [ ] Test installers on clean machines
- [ ] Update release notes on GitHub

## 📞 Getting Help

| Topic | Resource |
|-------|----------|
| Meta-features | [META_FEATURES.md](META_FEATURES.md) |
| Setup guide | [SETUP_GUIDE.md](SETUP_GUIDE.md) |
| Implementation | [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md) |
| ImGui docs | https://github.com/ocornut/imgui |
| CMake docs | https://cmake.org/documentation/ |
| GitHub Actions | https://docs.github.com/en/actions |

## ⚡ Tips

- Use VS Code (configs included)
- Tag releases with semantic versioning (v1.2.3)
- Run `ctest` before tagging
- Use feature branches
- Monitor GitHub Actions
- Read META_FEATURES.md for details


