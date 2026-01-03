// Main test file for Catch2
// This file contains the main() function for the test executable

// In Catch2 v3, we provide main() explicitly
#include <catch2/catch_session.hpp>

int main(int argc, char* argv[]) {
    // This provides the default main that runs all tests
    return Catch::Session().run(argc, argv);
}
