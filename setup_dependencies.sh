#!/bin/bash

echo "Setting up MetaImGUI dependencies..."

# Create external directory
mkdir -p external

# Download and setup ImGui
if [ ! -d "external/imgui" ]; then
    echo "Downloading ImGui..."
    cd external
    git clone https://github.com/ocornut/imgui.git
    cd imgui
    git checkout v1.92.4
    cd ../..
    echo "ImGui downloaded successfully"
else
    echo "ImGui already exists"
fi

# Download and setup Catch2 (for testing)
if [ ! -d "external/catch2" ]; then
    echo "Downloading Catch2..."
    cd external
    git clone https://github.com/catchorg/Catch2.git catch2
    cd catch2
    git checkout v3.4.0
    cd ../..
    echo "Catch2 downloaded successfully"
else
    echo "Catch2 already exists"
fi

# Download and setup nlohmann/json (JSON library)
if [ ! -d "external/json" ]; then
    echo "Downloading nlohmann/json..."
    cd external
    git clone https://github.com/nlohmann/json.git
    cd json
    git checkout v3.11.3
    cd ../..
    echo "nlohmann/json downloaded successfully"
else
    echo "nlohmann/json already exists"
fi

# Download and setup ImPlot
if [ ! -d "external/implot" ]; then
    echo "Downloading ImPlot..."
    cd external
    git clone https://github.com/epezent/implot.git
    cd implot
    git checkout v0.17
    cd ../..
    echo "ImPlot downloaded successfully"
else
    echo "ImPlot already exists"
fi

# Install required system packages (Ubuntu/Debian)
if command -v apt-get &> /dev/null; then
    echo "Installing system dependencies (Ubuntu/Debian)..."
    sudo apt-get update
    sudo apt-get install -y \
        build-essential \
        cmake \
        libglfw3-dev \
        libgl1-mesa-dev \
        libglu1-mesa-dev \
        xorg-dev
fi

# Install required system packages (Fedora/RedHat)
if command -v dnf &> /dev/null; then
    echo "Installing system dependencies (Fedora)..."
    sudo dnf install -y \
        gcc-c++ \
        cmake \
        glfw-devel \
        mesa-libGL-devel \
        mesa-libGLU-devel \
        libXrandr-devel \
        libXinerama-devel \
        libXcursor-devel \
        libXi-devel
fi

# Create build directory
mkdir -p build

# Install pre-commit hook for automatic code formatting
echo ""
echo "📝 Installing pre-commit hook..."
if [ -f ".pre-commit-hook.sh" ]; then
    cp .pre-commit-hook.sh .git/hooks/pre-commit
    chmod +x .git/hooks/pre-commit
    echo "✅ Pre-commit hook installed (automatic code formatting before commits)"
else
    echo "⚠️  Pre-commit hook script not found"
fi

echo ""
echo "Dependencies setup complete!"
echo "To build:"
echo "  cd build"
echo "  cmake .."
echo "  make"
