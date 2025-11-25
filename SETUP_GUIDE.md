# MetaImGUI Setup Guide

Setup instructions for using MetaImGUI as a template.

## 🎯 What's Included

- ✅ Cross-platform ImGui application
- ✅ CI/CD workflows
- ✅ Installers (AppImage, .deb, NSIS, DMG)
- ✅ Update notifications
- ✅ Testing framework
- ✅ Code quality tools
- ✅ Packaging scripts

## 📋 Prerequisites

### All Platforms
- Git
- CMake 3.16+
- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019 16.11+)

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libglfw3-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libcurl4-openssl-dev \
    xorg-dev
```

### Linux (Fedora)
```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    git \
    glfw-devel \
    mesa-libGL-devel \
    mesa-libGLU-devel \
    libcurl-devel \
    libXrandr-devel \
    libXinerama-devel \
    libXcursor-devel \
    libXi-devel
```

### Windows
1. Install Visual Studio 2019+ with C++ tools
2. Install CMake: https://cmake.org/download/
3. Install vcpkg:
   ```powershell
   git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
   cd C:\vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   ```

### macOS
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake glfw
```

## 🚀 Setup Steps

### Step 1: Clone the Template

```bash
# Clone into your new project directory
git clone https://github.com/yourusername/MetaImGUI.git MyAwesomeApp
cd MyAwesomeApp
```

### Step 2: Initialize Your Project

**Interactive Mode (Recommended):**
```bash
# Linux/macOS
./init_template.sh

# Windows
.\init_template.ps1
```

The script will ask you for:
- Project name (e.g., "MyAwesomeApp")
- C++ namespace (e.g., "MyApp")
- Author name
- Project description
- GitHub username (for update checking)

**Non-Interactive Mode:**
```bash
./init_template.sh \
  --name "MyAwesomeApp" \
  --namespace "MyApp" \
  --author "John Doe" \
  --description "My awesome ImGui application" \
  --github-user "johndoe"
```

### Step 3: Setup Dependencies

```bash
# Linux/macOS
./setup_dependencies.sh

# Windows
.\setup_dependencies.bat
```

Downloads ImGui v1.92.4, Catch2 v3.4.0, nlohmann/json v3.11.3

### Step 4: Build the Project

```bash
# Linux/macOS
./build.sh

# Windows
.\build.bat
```

### Step 5: Run Your Application

```bash
# Linux/macOS
./build/MyAwesomeApp

# Windows
.\build\Release\MyAwesomeApp.exe
```

## 🔧 Configuration

### Update Checker Configuration

Edit `src/Application.cpp` and update the UpdateChecker initialization:

```cpp
// Replace with your GitHub username and repository name
m_updateChecker = std::make_unique<UpdateChecker>("your-github-username", "your-repo-name");
```

### GitHub Actions Setup

1. Push your code to GitHub
2. Go to your repository → Settings → Actions
3. Enable GitHub Actions
4. Push a tag to trigger your first release:
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```

### Adding a Project Icon

1. Create a 256x256 PNG icon
2. Replace placeholder icons in `packaging/` directory
3. Update icon paths in packaging scripts

## 📦 Creating Releases

### Automated (Recommended)

```bash
# Create and push a version tag
git tag v1.2.3
git push origin v1.2.3

# GitHub Actions will automatically:
# - Build for Linux, Windows, macOS
# - Create installers
# - Generate changelog
# - Publish GitHub Release
```

### Manual Packaging

**Linux:**
```bash
./packaging/create_linux_packages.sh 1.0.0
# Creates: AppImage, .deb, .tar.gz
```

**Windows:**
```powershell
.\packaging\create_windows_installer.ps1 -Version 1.0.0
# Creates: .msi installer, portable .zip
```

## 🧪 Running Tests

```bash
# Build with tests enabled (default)
cmake -B build -DBUILD_TESTS=ON
cmake --build build

# Run tests
cd build
ctest --output-on-failure

# Or run test executable directly
./build/MetaImGUI_tests
```

## 💎 Code Quality

```bash
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

## 🛠️ Development Environment

### Visual Studio Code (Recommended)

1. Open the project folder in VS Code
2. Install recommended extensions (prompt will appear)
3. Press F5 to build and debug
4. Ctrl+Shift+B to build

### CLion / Visual Studio

- Open the project root folder
- CMake configuration will be detected automatically
- Build and run using IDE controls

## 📁 Project Structure

```
MyAwesomeApp/
├── .github/workflows/   # CI/CD automation
├── .vscode/            # VS Code configuration
├── cmake/              # Custom CMake modules
├── external/           # Third-party dependencies
├── include/            # Header files
├── src/                # Source files
├── tests/              # Unit tests
├── packaging/          # Installer scripts
├── resources/          # Application resources
├── CMakeLists.txt      # Build configuration
├── init_template.sh    # Project initialization
└── META_FEATURES.md    # Detailed feature documentation
```

## 🐛 Troubleshooting

### Build Errors

**"ImGui not found"**
```bash
# Run setup script
./setup_dependencies.sh
```

**"GLFW not found"**
```bash
# Linux
sudo apt-get install libglfw3-dev

# Windows
vcpkg install glfw3:x64-windows

# macOS
brew install glfw
```

**"curl not found" (Linux)**
```bash
sudo apt-get install libcurl4-openssl-dev
```

### Runtime Errors

**"Cannot open display" (Linux)**
```bash
# Ensure X11 is running
export DISPLAY=:0
```

**"Missing DLL" (Windows)**
- Build type mismatch (Debug vs Release)
- Copy glfw3.dll from vcpkg to executable directory

## 📚 Next Steps

1. Edit `src/Application.cpp`
2. Add source files
3. Write tests in `tests/`
4. Update README.md
5. Add LICENSE file
6. Enable GitHub Actions
7. Tag v1.0.0

## 🆘 Getting Help

- **Template Documentation**: See [META_FEATURES.md](META_FEATURES.md)
- **ImGui Documentation**: https://github.com/ocornut/imgui
- **CMake Documentation**: https://cmake.org/documentation/
- **GitHub Actions**: https://docs.github.com/en/actions

## ✅ Production Checklist

- [ ] Run `init_template.sh`
- [ ] Update README.md
- [ ] Add LICENSE file
- [ ] Replace icons
- [ ] Configure UpdateChecker
- [ ] Test on target platforms
- [ ] Write tests
- [ ] Enable GitHub Actions
- [ ] Push release tag
- [ ] Test installers

