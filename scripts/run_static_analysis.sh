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
TIDY_FAILED=false
if [ "$CLANG_TIDY_AVAILABLE" = true ]; then
    echo ""
    echo "🔍 Running clang-tidy..."
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

    FILE_COUNT=$(find src include -name '*.cpp' -o -name '*.h' | wc -l)
    echo "  Analyzing $FILE_COUNT files (ignoring external headers)"
    echo ""

    # Create temporary file to store clang-tidy output
    TIDY_OUTPUT=$(mktemp)

    # Run clang-tidy ONLY on your source files, suppress external warnings
    find src include -name '*.cpp' -o -name '*.h' | while read -r file; do
        $CLANG_TIDY -p "$BUILD_DIR" \
            --config-file=.clang-tidy \
            --header-filter="$PROJECT_ROOT/(src|include)/.*" \
            "$file" 2>&1 | \
            grep -v "warnings generated" | \
            grep --color=never -E "^($PROJECT_ROOT/)?(src|include)/" || true
    done | tee "$TIDY_OUTPUT"

    # Count warnings and errors
    WARNING_COUNT=$(grep -c "warning:" "$TIDY_OUTPUT" || true)
    ERROR_COUNT=$(grep -c "error:" "$TIDY_OUTPUT" || true)

    # Clean up temp file
    rm -f "$TIDY_OUTPUT"

    echo ""
    if [ "$ERROR_COUNT" -gt 0 ] || [ "$WARNING_COUNT" -gt 0 ]; then
        echo "❌ clang-tidy found issues: $ERROR_COUNT error(s), $WARNING_COUNT warning(s)"
        TIDY_FAILED=true
    else
        echo "✅ clang-tidy analysis complete - no issues found"
    fi
fi

# Run cppcheck
CPPCHECK_FAILED=false
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
        CPPCHECK_FAILED=true
    fi
fi

echo ""
echo "=== Static Analysis Complete ===="
echo ""

# Exit with error if any tool found issues
if [ "$TIDY_FAILED" = true ] || [ "$CPPCHECK_FAILED" = true ]; then
    echo "❌ Static analysis found issues that need to be fixed"
    echo ""
    echo "To fix issues:"
    echo "  1. Review the warnings/errors above"
    echo "  2. Fix the code or add suppressions if needed"
    echo "  3. Run this script again to verify fixes"
    echo ""
    exit 1
else
    echo "✅ All static analysis checks passed!"
    echo ""
    echo "For CI enforcement, these checks run automatically on push/PR"
    exit 0
fi

