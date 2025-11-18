#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "ConfigManager.h"

#include <filesystem>

using namespace MetaImGUI;
using Catch::Matchers::WithinAbs;

TEST_CASE("ConfigManager basic functionality", "[config]") {
    ConfigManager config;

    SECTION("Default values are set") {
        auto size = config.GetWindowSize();
        REQUIRE(size.has_value());
        REQUIRE(size->first == 1200);
        REQUIRE(size->second == 800);

        REQUIRE(config.GetTheme() == "Modern");
        REQUIRE_FALSE(config.GetWindowMaximized());
    }

    SECTION("Window position can be set and retrieved") {
        config.SetWindowPosition(100, 200);
        auto pos = config.GetWindowPosition();
        REQUIRE(pos.has_value());
        REQUIRE(pos->first == 100);
        REQUIRE(pos->second == 200);
    }

    SECTION("Window size can be set and retrieved") {
        config.SetWindowSize(1920, 1080);
        auto size = config.GetWindowSize();
        REQUIRE(size.has_value());
        REQUIRE(size->first == 1920);
        REQUIRE(size->second == 1080);
    }

    SECTION("Window maximized state can be set") {
        config.SetWindowMaximized(true);
        REQUIRE(config.GetWindowMaximized());

        config.SetWindowMaximized(false);
        REQUIRE_FALSE(config.GetWindowMaximized());
    }

    SECTION("Theme can be changed") {
        config.SetTheme("Dark");
        REQUIRE(config.GetTheme() == "Dark");

        config.SetTheme("Light");
        REQUIRE(config.GetTheme() == "Light");
    }
}

TEST_CASE("ConfigManager string settings", "[config]") {
    ConfigManager config;

    SECTION("String values can be stored and retrieved") {
        config.SetString("test_key", "test_value");
        auto value = config.GetString("test_key");
        REQUIRE(value.has_value());
        REQUIRE(value.value() == "test_value");
    }

    SECTION("Non-existent keys return nullopt") {
        auto value = config.GetString("nonexistent");
        REQUIRE_FALSE(value.has_value());
    }

    SECTION("Keys can be checked for existence") {
        config.SetString("exists", "value");
        REQUIRE(config.HasKey("exists"));
        REQUIRE_FALSE(config.HasKey("does_not_exist"));
    }

    SECTION("Keys can be removed") {
        config.SetString("to_remove", "value");
        REQUIRE(config.HasKey("to_remove"));

        config.RemoveKey("to_remove");
        REQUIRE_FALSE(config.HasKey("to_remove"));
    }
}

TEST_CASE("ConfigManager numeric settings", "[config]") {
    ConfigManager config;

    SECTION("Integer values work correctly") {
        config.SetInt("int_key", 42);
        auto value = config.GetInt("int_key");
        REQUIRE(value.has_value());
        REQUIRE(value.value() == 42);
    }

    SECTION("Boolean values work correctly") {
        config.SetBool("bool_key", true);
        auto value = config.GetBool("bool_key");
        REQUIRE(value.has_value());
        REQUIRE(value.value() == true);

        config.SetBool("bool_key", false);
        value = config.GetBool("bool_key");
        REQUIRE(value.has_value());
        REQUIRE(value.value() == false);
    }

    SECTION("Float values work correctly") {
        config.SetFloat("float_key", 3.14f);
        auto value = config.GetFloat("float_key");
        REQUIRE(value.has_value());
        REQUIRE_THAT(value.value(), WithinAbs(3.14f, 0.001f));
    }
}

TEST_CASE("ConfigManager recent files", "[config]") {
    ConfigManager config;

    SECTION("Recent files can be added") {
        config.AddRecentFile("/path/to/file1.txt");
        config.AddRecentFile("/path/to/file2.txt");

        auto files = config.GetRecentFiles();
        REQUIRE(files.size() == 2);
        REQUIRE(files[0] == "/path/to/file2.txt"); // Most recent first
        REQUIRE(files[1] == "/path/to/file1.txt");
    }

    SECTION("Duplicate files are moved to front") {
        config.AddRecentFile("/path/to/file1.txt");
        config.AddRecentFile("/path/to/file2.txt");
        config.AddRecentFile("/path/to/file1.txt"); // Add again

        auto files = config.GetRecentFiles();
        REQUIRE(files.size() == 2);
        REQUIRE(files[0] == "/path/to/file1.txt");
        REQUIRE(files[1] == "/path/to/file2.txt");
    }

    SECTION("Recent files list is limited") {
        config.SetMaxRecentFiles(3);

        config.AddRecentFile("/file1.txt");
        config.AddRecentFile("/file2.txt");
        config.AddRecentFile("/file3.txt");
        config.AddRecentFile("/file4.txt"); // Should push out file1

        auto files = config.GetRecentFiles();
        REQUIRE(files.size() == 3);
        REQUIRE(files[0] == "/file4.txt");
        REQUIRE(files[1] == "/file3.txt");
        REQUIRE(files[2] == "/file2.txt");
    }

    SECTION("Recent files can be cleared") {
        config.AddRecentFile("/file1.txt");
        config.AddRecentFile("/file2.txt");
        REQUIRE(config.GetRecentFiles().size() == 2);

        config.ClearRecentFiles();
        REQUIRE(config.GetRecentFiles().size() == 0);
    }
}

TEST_CASE("ConfigManager persistence", "[config]") {
    // Create a temporary config for testing
    ConfigManager config1;

    SECTION("Configuration can be saved and loaded") {
        // Set some values
        config1.SetWindowPosition(150, 250);
        config1.SetWindowSize(1600, 900);
        config1.SetTheme("Dark");
        config1.SetString("custom", "value");
        config1.AddRecentFile("/test/file.txt");

        // Save
        bool saved = config1.Save();
        REQUIRE(saved);
        REQUIRE(config1.ConfigFileExists());

        // Create new config and load
        ConfigManager config2;
        bool loaded = config2.Load();
        REQUIRE(loaded);

        // Verify values
        auto pos = config2.GetWindowPosition();
        REQUIRE(pos.has_value());
        REQUIRE(pos->first == 150);
        REQUIRE(pos->second == 250);

        auto size = config2.GetWindowSize();
        REQUIRE(size.has_value());
        REQUIRE(size->first == 1600);
        REQUIRE(size->second == 900);

        REQUIRE(config2.GetTheme() == "Dark");

        auto custom = config2.GetString("custom");
        REQUIRE(custom.has_value());
        REQUIRE(custom.value() == "value");

        auto files = config2.GetRecentFiles();
        REQUIRE(files.size() == 1);
        REQUIRE(files[0] == "/test/file.txt");
    }

    SECTION("Reset clears configuration") {
        config1.SetWindowPosition(100, 100);
        config1.SetTheme("Custom");
        config1.AddRecentFile("/file.txt");

        config1.Reset();

        // Should be back to defaults
        auto size = config1.GetWindowSize();
        REQUIRE(size.has_value());
        REQUIRE(size->first == 1200);
        REQUIRE(size->second == 800);

        REQUIRE(config1.GetTheme() == "Modern");
        REQUIRE(config1.GetRecentFiles().empty());
    }
}

TEST_CASE("ConfigManager key enumeration", "[config]") {
    ConfigManager config;

    SECTION("All keys can be enumerated") {
        config.SetString("key1", "value1");
        config.SetInt("key2", 42);
        config.SetBool("key3", true);

        auto keys = config.GetAllKeys();
        REQUIRE(keys.size() == 3);

        // Check that all keys are present (order not guaranteed)
        bool hasKey1 = false, hasKey2 = false, hasKey3 = false;
        for (const auto& key : keys) {
            if (key == "key1") hasKey1 = true;
            if (key == "key2") hasKey2 = true;
            if (key == "key3") hasKey3 = true;
        }
        REQUIRE(hasKey1);
        REQUIRE(hasKey2);
        REQUIRE(hasKey3);
    }
}

