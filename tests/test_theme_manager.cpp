#include "ThemeManager.h"

#include <catch2/catch_test_macros.hpp>

using namespace MetaImGUI;

// Note: ThemeManager tests are limited because ThemeManager::Apply() requires
// a valid ImGui context which is not available in unit tests without a window.
// In a real application, these themes are applied after ImGui initialization.
// These tests verify the enum structure and basic API design.

TEST_CASE("ThemeManager API design", "[theme]") {
    SECTION("Theme enum values are accessible") {
        // Verify all theme enums exist and are accessible
        auto dark = ThemeManager::Theme::Dark;
        auto light = ThemeManager::Theme::Light;
        auto classic = ThemeManager::Theme::Classic;
        auto modern = ThemeManager::Theme::Modern;

        // Just verifying compilation and enum access
        REQUIRE(true);
        (void)dark;
        (void)light;
        (void)classic;
        (void)modern;
    }
}

TEST_CASE("ThemeManager enum values", "[theme]") {
    SECTION("All theme enum values are unique") {
        REQUIRE(ThemeManager::Theme::Dark != ThemeManager::Theme::Light);
        REQUIRE(ThemeManager::Theme::Dark != ThemeManager::Theme::Classic);
        REQUIRE(ThemeManager::Theme::Dark != ThemeManager::Theme::Modern);
        REQUIRE(ThemeManager::Theme::Light != ThemeManager::Theme::Classic);
        REQUIRE(ThemeManager::Theme::Light != ThemeManager::Theme::Modern);
        REQUIRE(ThemeManager::Theme::Classic != ThemeManager::Theme::Modern);
    }
}
