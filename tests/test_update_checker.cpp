#include "UpdateChecker.h"

#include <catch2/catch_test_macros.hpp>

using namespace MetaImGUI;

TEST_CASE("UpdateChecker version comparison", "[version]") {
    SECTION("Equal versions") {
        REQUIRE(UpdateChecker::CompareVersions("1.0.0", "1.0.0") == 0);
        REQUIRE(UpdateChecker::CompareVersions("2.5.3", "2.5.3") == 0);
    }

    SECTION("First version is less") {
        REQUIRE(UpdateChecker::CompareVersions("1.0.0", "1.0.1") < 0);
        REQUIRE(UpdateChecker::CompareVersions("1.0.0", "2.0.0") < 0);
        REQUIRE(UpdateChecker::CompareVersions("1.5.0", "1.6.0") < 0);
    }

    SECTION("First version is greater") {
        REQUIRE(UpdateChecker::CompareVersions("1.0.1", "1.0.0") > 0);
        REQUIRE(UpdateChecker::CompareVersions("2.0.0", "1.0.0") > 0);
        REQUIRE(UpdateChecker::CompareVersions("1.6.0", "1.5.0") > 0);
    }

    SECTION("Version with 'v' prefix") {
        REQUIRE(UpdateChecker::CompareVersions("v1.0.0", "1.0.0") == 0);
        REQUIRE(UpdateChecker::CompareVersions("1.0.0", "v1.0.0") == 0);
    }

    SECTION("Versions with different number of parts") {
        REQUIRE(UpdateChecker::CompareVersions("1.0", "1.0.0") == 0);
        REQUIRE(UpdateChecker::CompareVersions("1.0.0", "1.0") == 0);
    }

    SECTION("SemVer pre-release ranks below the release") {
        REQUIRE(UpdateChecker::CompareVersions("1.2.0-rc1", "1.2.0") < 0);
        REQUIRE(UpdateChecker::CompareVersions("1.2.0", "1.2.0-rc1") > 0);
        REQUIRE(UpdateChecker::CompareVersions("1.2.0-alpha", "1.2.0-beta") < 0);
        REQUIRE(UpdateChecker::CompareVersions("1.2.0-rc.1", "1.2.0-rc.2") < 0);
    }

    SECTION("SemVer numeric identifiers compare numerically") {
        REQUIRE(UpdateChecker::CompareVersions("1.0.0-alpha.2", "1.0.0-alpha.10") < 0);
    }

    SECTION("SemVer numeric identifiers rank below alphanumeric") {
        REQUIRE(UpdateChecker::CompareVersions("1.0.0-1", "1.0.0-alpha") < 0);
    }

    SECTION("SemVer fewer pre-release identifiers ranks below more (when prefix matches)") {
        REQUIRE(UpdateChecker::CompareVersions("1.0.0-alpha", "1.0.0-alpha.1") < 0);
    }

    SECTION("SemVer build metadata is ignored") {
        REQUIRE(UpdateChecker::CompareVersions("1.2.3+build.5", "1.2.3+build.42") == 0);
        REQUIRE(UpdateChecker::CompareVersions("1.2.3-rc1+build.5", "1.2.3-rc1+build.42") == 0);
    }

    SECTION("Cross-version SemVer ordering") {
        REQUIRE(UpdateChecker::CompareVersions("1.2.0-rc1", "1.2.0") < 0);
        REQUIRE(UpdateChecker::CompareVersions("1.2.0", "1.2.1-rc1") < 0);
        REQUIRE(UpdateChecker::CompareVersions("1.2.1-rc1", "1.2.1") < 0);
    }
}

TEST_CASE("UpdateChecker basic functionality", "[update]") {
    UpdateChecker checker("test-owner", "test-repo");

    SECTION("Check if checking state works") {
        REQUIRE_FALSE(checker.IsChecking());
    }

    // Note: We don't test actual network requests in unit tests
    // Those should be integration tests
}
