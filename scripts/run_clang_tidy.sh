#!/bin/bash
# Script to run clang-tidy on all source files
# Arguments: $1 = build directory, $2 = clang-tidy executable, $3 = config file

set -e

BUILD_DIR="${1:-.}"
CLANG_TIDY="${2:-clang-tidy}"
CONFIG_FILE="${3:-.clang-tidy}"

SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "Running clang-tidy from: $SOURCE_DIR"
echo "Build directory: $BUILD_DIR"
echo "Config file: $CONFIG_FILE"

cd "$SOURCE_DIR"

# Find all .cpp and .h files in src and include directories
find src include -type f \( -name "*.cpp" -o -name "*.h" \) \
    -exec "$CLANG_TIDY" -p "$BUILD_DIR" --config-file="$CONFIG_FILE" {} +

