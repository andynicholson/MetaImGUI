#!/bin/bash
# Post-create script for devcontainer
# Runs after the container is created

set -e

echo "🚀 Setting up MetaImGUI development environment..."

# Setup dependencies
echo "📦 Setting up project dependencies..."
if [ -f "./setup_dependencies.sh" ]; then
    chmod +x ./setup_dependencies.sh
    ./setup_dependencies.sh
fi

# Configure git if not already configured
if [ -z "$(git config --global user.name)" ]; then
    echo "⚙️  Configuring git..."
    git config --global --add safe.directory /workspace
    echo "ℹ️  Remember to set git user.name and user.email:"
    echo "   git config --global user.name \"Your Name\""
    echo "   git config --global user.email \"your.email@example.com\""
fi

# Create build directory structure
echo "📁 Creating build directories..."
mkdir -p build
mkdir -p logs

# Configure CMake with useful defaults
echo "🔨 Configuring CMake..."
cmake -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DBUILD_TESTS=ON \
    -DBUILD_BENCHMARKS=OFF \
    -DENABLE_COVERAGE=OFF

# Create compile_commands.json symlink for clangd
if [ ! -f "compile_commands.json" ] && [ -f "build/compile_commands.json" ]; then
    ln -s build/compile_commands.json compile_commands.json
fi

# Set up pre-commit hook
if [ -f ".pre-commit-hook.sh" ] && [ ! -f ".git/hooks/pre-commit" ]; then
    echo "🪝 Installing pre-commit hook..."
    cp .pre-commit-hook.sh .git/hooks/pre-commit
    chmod +x .git/hooks/pre-commit
fi

echo "✅ Development environment setup complete!"
echo ""
echo "Quick commands:"
echo "  Build:        cmake --build build"
echo "  Test:         cd build && ctest --output-on-failure"
echo "  Coverage:     ./scripts/run_coverage.sh"
echo "  Sanitizers:   ./scripts/run_sanitizers.sh"
echo "  Format:       find src include -name '*.cpp' -o -name '*.h' | xargs clang-format -i"
echo ""
echo "Happy coding! 🎉"

