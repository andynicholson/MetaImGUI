#!/bin/bash
# Script to generate code coverage report locally

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== MetaImGUI Code Coverage Generator ===${NC}"

# Check for required tools
if ! command -v lcov &> /dev/null; then
    echo -e "${RED}Error: lcov not found${NC}"
    echo "Install with:"
    echo "  Ubuntu/Debian: sudo apt-get install lcov"
    echo "  macOS: brew install lcov"
    echo "  Fedora: sudo dnf install lcov"
    exit 1
fi

if ! command -v genhtml &> /dev/null; then
    echo -e "${RED}Error: genhtml not found (should come with lcov)${NC}"
    exit 1
fi

# Check if build directory exists
if [ ! -d "build" ]; then
    echo -e "${YELLOW}Build directory not found. Creating...${NC}"
    mkdir build
fi

cd build

# Configure with coverage enabled
echo -e "${GREEN}Configuring with coverage enabled...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -DBUILD_TESTS=ON

# Build
echo -e "${GREEN}Building...${NC}"
cmake --build . --parallel

# Zero coverage counters
echo -e "${GREEN}Resetting coverage counters...${NC}"
lcov --directory . --zerocounters

# Run tests
echo -e "${GREEN}Running tests...${NC}"
ctest --output-on-failure || {
    echo -e "${RED}Tests failed. Coverage may be incomplete.${NC}"
}

# Capture coverage data
echo -e "${GREEN}Capturing coverage data...${NC}"
lcov --directory . --capture --output-file coverage.info --ignore-errors source,gcov,negative

# Remove external and test files
echo -e "${GREEN}Filtering coverage data...${NC}"
lcov --remove coverage.info \
    '/usr/*' \
    '*/external/*' \
    '*/tests/*' \
    '*/build/*' \
    --output-file coverage.info.cleaned \
    --ignore-errors source,gcov,empty,negative

# Generate HTML report
echo -e "${GREEN}Generating HTML report...${NC}"
genhtml coverage.info.cleaned --output-directory coverage_html

# Print summary
echo -e "${GREEN}=== Coverage Summary ===${NC}"
lcov --summary coverage.info.cleaned

# Open report
echo ""
echo -e "${GREEN}Coverage report generated!${NC}"
echo -e "HTML report: ${YELLOW}build/coverage_html/index.html${NC}"
echo ""
echo "To view the report:"
echo "  - Linux: xdg-open build/coverage_html/index.html"
echo "  - macOS: open build/coverage_html/index.html"
echo "  - Windows: start build/coverage_html/index.html"

# Optionally open in browser
if command -v xdg-open &> /dev/null; then
    read -p "Open report in browser? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        xdg-open coverage_html/index.html
    fi
elif command -v open &> /dev/null; then
    read -p "Open report in browser? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        open coverage_html/index.html
    fi
fi

