#!/bin/bash
# Script to run tests with various sanitizers

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== MetaImGUI Sanitizer Test Runner ===${NC}"

# Check for compiler
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo -e "${RED}Error: No suitable C++ compiler found${NC}"
    exit 1
fi

# Determine which sanitizers to run
SANITIZERS=()
if [ "$#" -eq 0 ]; then
    echo -e "${YELLOW}No sanitizer specified. Running all sanitizers...${NC}"
    SANITIZERS=("ASAN" "TSAN" "UBSAN")
else
    for arg in "$@"; do
        case "${arg^^}" in
            ASAN|ADDRESS)
                SANITIZERS+=("ASAN")
                ;;
            TSAN|THREAD)
                SANITIZERS+=("TSAN")
                ;;
            UBSAN|UNDEFINED)
                SANITIZERS+=("UBSAN")
                ;;
            MSAN|MEMORY)
                SANITIZERS+=("MSAN")
                ;;
            ALL)
                SANITIZERS=("ASAN" "TSAN" "UBSAN")
                ;;
            *)
                echo -e "${RED}Unknown sanitizer: $arg${NC}"
                echo "Valid options: ASAN, TSAN, UBSAN, MSAN, ALL"
                exit 1
                ;;
        esac
    done
fi

# Run each sanitizer
FAILED_SANITIZERS=()

for SANITIZER in "${SANITIZERS[@]}"; do
    echo ""
    echo -e "${BLUE}======================================${NC}"
    echo -e "${BLUE}Running with $SANITIZER${NC}"
    echo -e "${BLUE}======================================${NC}"

    BUILD_DIR="build_${SANITIZER,,}"

    # Clean up previous build
    if [ -d "$BUILD_DIR" ]; then
        echo -e "${YELLOW}Removing previous build directory: $BUILD_DIR${NC}"
        rm -rf "$BUILD_DIR"
    fi

    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # Configure
    echo -e "${GREEN}Configuring with $SANITIZER...${NC}"
    if ! cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_${SANITIZER}=ON -DBUILD_TESTS=ON; then
        echo -e "${RED}Configuration failed for $SANITIZER${NC}"
        FAILED_SANITIZERS+=("$SANITIZER")
        cd ..
        continue
    fi

    # Build
    echo -e "${GREEN}Building...${NC}"
    if ! cmake --build . --parallel; then
        echo -e "${RED}Build failed for $SANITIZER${NC}"
        FAILED_SANITIZERS+=("$SANITIZER")
        cd ..
        continue
    fi

    # Set sanitizer options
    case "$SANITIZER" in
        ASAN)
            export ASAN_OPTIONS="detect_leaks=1:check_initialization_order=1:strict_init_order=1:halt_on_error=0"
            ;;
        TSAN)
            export TSAN_OPTIONS="second_deadlock_stack=1:halt_on_error=0"
            ;;
        UBSAN)
            export UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=0"
            ;;
        MSAN)
            export MSAN_OPTIONS="halt_on_error=0"
            ;;
    esac

    # Run tests
    echo -e "${GREEN}Running tests with $SANITIZER...${NC}"
    if ! ctest --output-on-failure --verbose; then
        echo -e "${RED}Tests failed with $SANITIZER${NC}"
        FAILED_SANITIZERS+=("$SANITIZER")
    else
        echo -e "${GREEN}✓ $SANITIZER tests passed!${NC}"
    fi

    cd ..
done

# Summary
echo ""
echo -e "${BLUE}======================================${NC}"
echo -e "${BLUE}Summary${NC}"
echo -e "${BLUE}======================================${NC}"

if [ ${#FAILED_SANITIZERS[@]} -eq 0 ]; then
    echo -e "${GREEN}✓ All sanitizer tests passed!${NC}"
    exit 0
else
    echo -e "${RED}✗ Failed sanitizers:${NC}"
    for SANITIZER in "${FAILED_SANITIZERS[@]}"; do
        echo -e "${RED}  - $SANITIZER${NC}"
    done
    exit 1
fi

