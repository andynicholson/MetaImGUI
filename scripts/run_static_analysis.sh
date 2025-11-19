#!/bin/bash

# Script to run static analysis tools locally
# Runs clang-tidy and cppcheck on the codebase

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_ROOT}/build"

echo "=== MetaImGUI Static Analysis ===="
echo ""

# Check for required tools
CLANG_TIDY=$(command -v clang-tidy-18 || command -v clang-tidy-17 || command -v clang-tidy || echo "")
CPPCHECK=$(command -v cppcheck || echo "")

if [ -z "$CLANG_TIDY" ]; then
    echo "⚠️  clang-tidy not found. Install with:"
    echo "    Ubuntu: sudo apt install clang-tidy"
    echo "    macOS:  brew install llvm"
    CLANG_TIDY_AVAILABLE=false
else
    echo "✓ clang-tidy found: $CLANG_TIDY"
    CLANG_TIDY_AVAILABLE=true
fi

if [ -z "$CPPCHECK" ]; then
    echo "⚠️  cppcheck not found. Install with:"
    echo "    Ubuntu: sudo apt install cppcheck"
    echo "    macOS:  brew install cppcheck"
    CPPCHECK_AVAILABLE=false
else
    echo "✓ cppcheck found: $CPPCHECK"
    CPPCHECK_AVAILABLE=true
fi

echo ""

if [ "$CLANG_TIDY_AVAILABLE" = false ] && [ "$CPPCHECK_AVAILABLE" = false ]; then
    echo "❌ No static analysis tools available. Please install clang-tidy and/or cppcheck."
    exit 1
fi

# Ensure build directory exists with compile_commands.json
if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "📦 Build directory not configured. Running CMake..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
fi

cd "$PROJECT_ROOT"

# Run clang-tidy
if [ "$CLANG_TIDY_AVAILABLE" = true ]; then
    echo ""
    echo "🔍 Running clang-tidy..."
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

    CLANG_TIDY_FAILED=false
    find src include -name '*.cpp' -o -name '*.h' | while read -r file; do
        echo "  Analyzing: $file"
        if ! $CLANG_TIDY -p "$BUILD_DIR" \
            --config-file=.clang-tidy \
            --header-filter='(include|src)/.*' \
            --quiet \
            "$file"; then
            CLANG_TIDY_FAILED=true
        fi
    done

    if [ "$CLANG_TIDY_FAILED" = true ]; then
        echo "❌ clang-tidy found issues"
    else
        echo "✅ clang-tidy passed"
    fi
fi

# Run cppcheck
if [ "$CPPCHECK_AVAILABLE" = true ]; then
    echo ""
    echo "🔍 Running cppcheck..."
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

    if $CPPCHECK \
        --enable=all \
        --inconclusive \
        --force \
        --inline-suppr \
        --suppressions-list=.cppcheck-suppressions \
        --std=c++17 \
        --language=c++ \
        --platform=unix64 \
        --template='{file}:{line}:{column}: {severity}: {message} [{id}]' \
        -I include \
        -I external/imgui \
        -I external/json/include \
        --suppress=missingIncludeSystem \
        --suppress=unmatchedSuppression \
        --suppress=unusedFunction \
        src/ include/; then
        echo "✅ cppcheck passed"
    else
        echo "❌ cppcheck found issues"
    fi
fi

echo ""
echo "=== Static Analysis Complete ===="
echo ""
echo "To fix issues:"
echo "  1. Review the warnings/errors above"
echo "  2. Fix the code or add suppressions if needed"
echo "  3. Run this script again to verify fixes"
echo ""
echo "For CI enforcement, these checks run automatically on push/PR"

