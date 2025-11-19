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
}

TEST_CASE("UpdateChecker basic functionality", "[update]") {
    UpdateChecker checker("test-owner", "test-repo");

    SECTION("Check if checking state works") {
        REQUIRE_FALSE(checker.IsChecking());
    }

    // Note: We don't test actual network requests in unit tests
    // Those should be integration tests
}
