#!/bin/bash
# Test script for Linux packaging formats
# Tests basic build functionality for each packaging format

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }

VERSION=${1:-"1.0.0"}
RESULTS=()

echo ""
echo "========================================"
echo "  MetaImGUI Package Testing"
echo "  Version: $VERSION"
echo "========================================"
echo ""

# Ensure we're in the project root
if [ ! -f "CMakeLists.txt" ]; then
    print_error "Please run this script from the project root directory"
    exit 1
fi

# Test AppImage and .deb (via create_linux_packages.sh)
test_linux_packages() {
    print_info "Testing AppImage and .deb creation..."

    if [ -f "packaging/create_linux_packages.sh" ]; then
        chmod +x packaging/create_linux_packages.sh

        if ./packaging/create_linux_packages.sh "$VERSION" 2>&1 | tee /tmp/linux_packages.log; then
            print_success "AppImage and .deb packages built successfully"
            RESULTS+=("✅ AppImage: PASS")
            RESULTS+=("✅ .deb: PASS")

            # Show created files
            if [ -d "packaging/output" ]; then
                echo ""
                print_info "Created packages:"
                ls -lh packaging/output/ | grep -v "^total" | awk '{print "  - " $9 " (" $5 ")"}'
                echo ""
            fi
        else
            print_error "Linux packages build failed"
            RESULTS+=("❌ AppImage: FAIL")
            RESULTS+=("❌ .deb: FAIL")
            print_info "Check /tmp/linux_packages.log for details"
        fi
    else
        print_warning "create_linux_packages.sh not found"
        RESULTS+=("⚠️  AppImage: SKIP")
        RESULTS+=("⚠️  .deb: SKIP")
    fi
}

# Test Flatpak
test_flatpak() {
    print_info "Testing Flatpak build..."

    if ! command -v flatpak-builder &> /dev/null; then
        print_warning "flatpak-builder not installed, skipping"
        RESULTS+=("⚠️  Flatpak: SKIP (flatpak-builder not installed)")
        return
    fi

    # Check for required runtime
    if ! flatpak list --runtime | grep -q "org.freedesktop.Platform.*23.08"; then
        print_warning "Flatpak runtime not installed"
        print_info "Install with: flatpak install flathub org.freedesktop.Platform//23.08 org.freedesktop.Sdk//23.08"
        RESULTS+=("⚠️  Flatpak: SKIP (runtime not installed)")
        return
    fi

    cd packaging/flatpak

    if flatpak-builder --force-clean --disable-download build-dir com.metaimgui.MetaImGUI.yaml 2>&1 | tee /tmp/flatpak_build.log; then
        print_success "Flatpak build successful"
        RESULTS+=("✅ Flatpak: PASS")
    else
        print_error "Flatpak build failed"
        RESULTS+=("❌ Flatpak: FAIL")
        print_info "Check /tmp/flatpak_build.log for details"
    fi

    cd ../..
}

# Test Snap
test_snap() {
    print_info "Testing Snap build..."

    if ! command -v snapcraft &> /dev/null; then
        print_warning "snapcraft not installed, skipping"
        RESULTS+=("⚠️  Snap: SKIP (snapcraft not installed)")
        return
    fi

    # Check if we're in a container or VM (snapcraft doesn't work well in all environments)
    if [ -f "/.dockerenv" ] || grep -q "microsoft" /proc/version 2>/dev/null; then
        print_warning "Running in container/WSL, skipping snap build"
        RESULTS+=("⚠️  Snap: SKIP (container/WSL environment)")
        return
    fi

    cd packaging/snap

    # Clean previous builds
    snapcraft clean 2>/dev/null || true

    if snapcraft --use-lxd 2>&1 | tee /tmp/snap_build.log; then
        print_success "Snap build successful"
        RESULTS+=("✅ Snap: PASS")
    else
        print_error "Snap build failed"
        RESULTS+=("❌ Snap: FAIL")
        print_info "Check /tmp/snap_build.log for details"
        print_info "Note: Snap builds require LXD. Install with: sudo snap install lxd"
    fi

    cd ../..
}

# Run tests
echo "Starting package tests..."
echo ""

test_linux_packages
echo ""

test_flatpak
echo ""

test_snap
echo ""

# Summary
echo ""
echo "========================================"
echo "  Test Summary"
echo "========================================"
echo ""

for result in "${RESULTS[@]}"; do
    echo "$result"
done

echo ""

# Count results
PASS_COUNT=$(printf '%s\n' "${RESULTS[@]}" | grep -c "✅" || true)
FAIL_COUNT=$(printf '%s\n' "${RESULTS[@]}" | grep -c "❌" || true)
SKIP_COUNT=$(printf '%s\n' "${RESULTS[@]}" | grep -c "⚠️" || true)

print_info "Results: $PASS_COUNT passed, $FAIL_COUNT failed, $SKIP_COUNT skipped"
echo ""

if [ "$FAIL_COUNT" -gt 0 ]; then
    print_error "Some tests failed!"
    exit 1
else
    print_success "All available tests passed!"
    if [ "$SKIP_COUNT" -gt 0 ]; then
        print_info "Some tests were skipped due to missing tools"
    fi
fi

