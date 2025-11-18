#!/bin/bash
# Pre-commit hook for code formatting
# Copy this to .git/hooks/pre-commit to enable automatic formatting

# Find all staged C++ files
FILES=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|h|hpp|cc|cxx)$')

if [ -z "$FILES" ]; then
    exit 0
fi

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    echo "Warning: clang-format is not installed. Skipping code formatting."
    echo "Install it with: sudo apt-get install clang-format"
    exit 0
fi

# Format files
echo "Running clang-format on staged files..."
for FILE in $FILES; do
    if [ -f "$FILE" ]; then
        echo "  Formatting: $FILE"
        clang-format -i "$FILE"
        git add "$FILE"
    fi
done

echo "Code formatting complete!"
exit 0

