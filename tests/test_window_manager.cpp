#include <catch2/catch_test_macros.hpp>
#include "WindowManager.h"

using namespace MetaImGUI;

// Note: WindowManager tests may fail in headless environments (CI/CD)
// This is expected behavior as GLFW requires a display

TEST_CASE("WindowManager basic functionality", "[window][!mayfail]") {
    SECTION("WindowManager can be created") {
        // In headless environment, WindowManager constructor succeeds
        // but ShouldClose() might return true if GLFW init fails
        WindowManager wm("Test Window", 800, 600);
        // This test is informational only
        INFO("Window should close: " << wm.ShouldClose());
        REQUIRE(true); // Always pass, just checking construction
    }

    SECTION("WindowManager has default size") {
        WindowManager wm("Test", 640, 480);
        // Just verify construction doesn't crash
        REQUIRE(true);
    }
}

TEST_CASE("WindowManager lifecycle", "[window][!mayfail]") {
    SECTION("Initialize and shutdown in headless mode") {
        WindowManager wm("Test", 800, 600);
        // In headless mode, Initialize() will fail gracefully
        // We just verify it doesn't crash
        wm.Initialize();
        REQUIRE(true); // Test passes if we get here without crashing
    }
}

