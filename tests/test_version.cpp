#include <catch2/catch_test_macros.hpp>
#include "version.h"
#include <string>
#include <cstring>

using namespace MetaImGUI;

TEST_CASE("Version information is available", "[version]") {
    SECTION("Version constants are defined") {
        REQUIRE(Version::MAJOR >= 0);
        REQUIRE(Version::MINOR >= 0);
        REQUIRE(Version::PATCH >= 0);
    }

    SECTION("Version strings are not empty") {
        REQUIRE(std::strlen(Version::VERSION) > 0);
        REQUIRE(std::strlen(Version::PROJECT) > 0);
    }

    SECTION("Project name is correct") {
        REQUIRE(std::string(Version::PROJECT) == "MetaImGUI");
    }

    SECTION("Build configuration is set") {
        // BUILD_CONFIG may be empty if CMAKE_BUILD_TYPE wasn't specified
        // This is acceptable, so we just verify the pointer is valid
        REQUIRE(Version::BUILD_CONFIG != nullptr);
        // Typically it should be "Debug", "Release", "RelWithDebInfo", or "MinSizeRel"
        // but it can also be empty string if not set during CMake configure
    }
}

