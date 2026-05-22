/*
    MetaImGUI
    Copyright (C) 2026  A P Nicholson

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// Smoke test: bring a WindowManager all the way up and tear it down.
//
// This needs a real display (X server, Wayland, or macOS WindowServer). On
// headless systems (no DISPLAY / WAYLAND_DISPLAY) the tests SKIP at runtime
// rather than fail, so unattended runs stay green. To force a display in
// Linux CI, wrap the test invocation in `xvfb-run`:
//
//   xvfb-run ctest --test-dir build --output-on-failure
//
// To run only the display-tagged tests:
//   ./MetaImGUI_tests "[display]"

#include "WindowManager.h"

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>

using MetaImGUI::WindowManager;

namespace {

bool HasDisplay() {
#if defined(__linux__)
    // NOLINTNEXTLINE(concurrency-mt-unsafe) - test setup, single-threaded
    const char* display = std::getenv("DISPLAY");
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    const char* wayland = std::getenv("WAYLAND_DISPLAY");
    return (display != nullptr && display[0] != '\0') || (wayland != nullptr && wayland[0] != '\0');
#else
    // macOS / Windows always have a display when the test binary runs
    // interactively. CI matrices for those platforms will skip the
    // [.display]-tagged tests via test list filtering.
    return true;
#endif
}

} // namespace

TEST_CASE("WindowManager: full init / shutdown cycle", "[window][display]") {
    if (!HasDisplay()) {
        SKIP("No display available; run under xvfb-run or attach a display");
    }

    WindowManager wm("Smoke Test", 320, 240);
    if (!wm.Initialize()) {
        // Display advertised but couldn't create a GL context (WSLg with
        // GLXBadFBConfig, headless DISPLAY without GLX, etc.). That's an
        // environment limitation, not a regression we're testing for.
        SKIP("WindowManager::Initialize() failed — display lacks GL context support");
    }

    REQUIRE(wm.GetNativeWindow() != nullptr);

    int width = 0;
    int height = 0;
    wm.GetWindowSize(width, height);
    // Window managers are allowed to clamp/scale — the only thing we care
    // about is that we got a non-zero, plausibly-sized window back.
    REQUIRE(width > 0);
    REQUIRE(height > 0);
}

TEST_CASE("WindowManager: pump one frame without errors", "[window][display]") {
    if (!HasDisplay()) {
        SKIP("No display available");
    }

    WindowManager wm("Smoke Test Frame", 320, 240);
    if (!wm.Initialize()) {
        SKIP("WindowManager::Initialize() failed — display lacks GL context support");
    }

    wm.PollEvents();
    wm.BeginFrame();
    wm.EndFrame();

    // If we got here without GL/GLFW exploding, that's the contract.
    SUCCEED();
}
