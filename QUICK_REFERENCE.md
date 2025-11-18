# MetaImGUI Quick Reference

One-page reference for common operations.

## 🚀 Quick Start

```bash
# Initialize new project
./init_template.sh

# Setup dependencies
./setup_dependencies.sh

# Build
./build.sh

# Run
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

### Code Quality

```bash
# Format all code
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Check formatting (dry run)
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run --Werror

# Enable pre-commit hook
cp .pre-commit-hook.sh .git/hooks/pre-commit && chmod +x .git/hooks/pre-commit
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

## ⚡ Pro Tips

1. **Use VS Code**: Best IDE support with included configs
2. **Enable pre-commit hook**: Automatic code formatting
3. **Tag releases properly**: Use semantic versioning (v1.2.3)
4. **Test before release**: Run `ctest` before tagging
5. **Keep git clean**: Commit often, push frequently
6. **Document changes**: Update README as you add features
7. **Use branches**: Develop features in branches, merge to main
8. **Monitor CI**: Check GitHub Actions after pushing
9. **Version carefully**: Tags are permanent, choose wisely
10. **Read META_FEATURES.md**: Detailed info on all features

---

**Need more help?** See the full documentation in the root directory!

