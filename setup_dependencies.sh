#!/bin/bash
set -euo pipefail

echo "Setting up MetaImGUI dependencies..."

# Pin third-party deps to commit SHAs, not tags. Tags can be re-pointed
# (intentionally by the maintainer, maliciously by an attacker who's
# compromised an upstream account); a SHA cannot. Each comment records the
# release tag the SHA corresponds to so a human reading the script can
# still tell at a glance which version we're on. To bump:
#   1. cd external/<dep> && git fetch --tags
#   2. git rev-parse <new-tag>
#   3. update both the tag comment and the SHA below
#   4. update the matching key in .github/workflows/ci.yml
IMGUI_SHA=9a5d5c45f54b1301ea471622eddede70384243af   # v1.92.4
JSON_SHA=9cca280a4d0ccf0c08f47a99aa71d1b0e52f8d03    # v3.11.3
IMPLOT_SHA=4707b245fbcd69075b1a8a74fa8d2435561b3134  # v0.17
CATCH2_SHA=f90e8f50a6e76e3492ee9aadfe1014677c4807d2  # v3.4.0

# Create external directory
mkdir -p external

clone_at_sha() {
    local name="$1"
    local repo="$2"
    local sha="$3"
    local dest="external/$name"

    if [ -d "$dest" ]; then
        echo "$name already exists"
        return 0
    fi

    echo "Downloading $name (pinned to $sha)..."
    git clone "$repo" "$dest"
    git -C "$dest" checkout --detach "$sha"
    echo "$name downloaded successfully"
}

clone_at_sha imgui  https://github.com/ocornut/imgui.git    "$IMGUI_SHA"
clone_at_sha catch2 https://github.com/catchorg/Catch2.git  "$CATCH2_SHA"
clone_at_sha json   https://github.com/nlohmann/json.git    "$JSON_SHA"
clone_at_sha implot https://github.com/epezent/implot.git   "$IMPLOT_SHA"

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
