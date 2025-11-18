#include <catch2/catch_test_macros.hpp>
#include "Logger.h"

#include <filesystem>
#include <fstream>
#include <thread>

using namespace MetaImGUI;

TEST_CASE("Logger basic functionality", "[logger]") {
    // Use a temporary log file for testing
    std::filesystem::path testLogPath = std::filesystem::temp_directory_path() / "metaimgui_test.log";

    SECTION("Logger singleton works") {
        Logger& logger1 = Logger::Instance();
        Logger& logger2 = Logger::Instance();
        REQUIRE(&logger1 == &logger2);
    }

    SECTION("Log levels can be set and retrieved") {
        Logger::Instance().SetLevel(LogLevel::Debug);
        REQUIRE(Logger::Instance().GetLevel() == LogLevel::Debug);

        Logger::Instance().SetLevel(LogLevel::Error);
        REQUIRE(Logger::Instance().GetLevel() == LogLevel::Error);

        // Reset to default
        Logger::Instance().SetLevel(LogLevel::Info);
    }

    SECTION("Console output can be toggled") {
        Logger::Instance().SetConsoleOutput(false);
        Logger::Instance().Info("This should not appear on console");
        Logger::Instance().SetConsoleOutput(true);
        Logger::Instance().Info("This should appear on console");
    }
}

TEST_CASE("Logger file output", "[logger]") {
    std::filesystem::path testLogPath = std::filesystem::temp_directory_path() / "metaimgui_test.log";

    // Clean up any existing test log
    if (std::filesystem::exists(testLogPath)) {
        std::filesystem::remove(testLogPath);
    }

    SECTION("Logger can write to file") {
        Logger::Instance().Initialize(testLogPath, LogLevel::Debug);
        Logger::Instance().Info("Test message");
        Logger::Instance().Flush();

        REQUIRE(std::filesystem::exists(testLogPath));

        // Read file contents
        std::ifstream file(testLogPath);
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        REQUIRE(content.find("Test message") != std::string::npos);
        REQUIRE(content.find("[INFO ]") != std::string::npos);

        Logger::Instance().Shutdown();
    }

    SECTION("Different log levels are written correctly") {
        Logger::Instance().Initialize(testLogPath, LogLevel::Debug);

        Logger::Instance().Debug("Debug message");
        Logger::Instance().Info("Info message");
        Logger::Instance().Warning("Warning message");
        Logger::Instance().Error("Error message");

        Logger::Instance().Flush();

        // Read file contents
        std::ifstream file(testLogPath);
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        REQUIRE(content.find("Debug message") != std::string::npos);
        REQUIRE(content.find("Info message") != std::string::npos);
        REQUIRE(content.find("Warning message") != std::string::npos);
        REQUIRE(content.find("Error message") != std::string::npos);

        REQUIRE(content.find("[DEBUG]") != std::string::npos);
        REQUIRE(content.find("[INFO ]") != std::string::npos);
        REQUIRE(content.find("[WARN ]") != std::string::npos);
        REQUIRE(content.find("[ERROR]") != std::string::npos);

        Logger::Instance().Shutdown();
    }

    SECTION("Log level filtering works") {
        Logger::Instance().Initialize(testLogPath, LogLevel::Warning);

        Logger::Instance().Debug("Should not appear");
        Logger::Instance().Info("Should not appear either");
        Logger::Instance().Warning("Should appear");
        Logger::Instance().Error("Should also appear");

        Logger::Instance().Flush();

        // Read file contents
        std::ifstream file(testLogPath);
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        REQUIRE(content.find("Should not appear") == std::string::npos);
        REQUIRE(content.find("Should appear") != std::string::npos);
        REQUIRE(content.find("Should also appear") != std::string::npos);

        Logger::Instance().Shutdown();
    }

    // Clean up
    if (std::filesystem::exists(testLogPath)) {
        std::filesystem::remove(testLogPath);
    }
}

TEST_CASE("Logger macros", "[logger]") {
    std::filesystem::path testLogPath = std::filesystem::temp_directory_path() / "metaimgui_test_macros.log";

    // Clean up
    if (std::filesystem::exists(testLogPath)) {
        std::filesystem::remove(testLogPath);
    }

    Logger::Instance().Initialize(testLogPath, LogLevel::Debug);

    SECTION("Convenience macros work") {
        LOG_DEBUG("Debug via macro");
        LOG_INFO("Info via macro");
        LOG_WARNING("Warning via macro");
        LOG_ERROR("Error via macro");

        Logger::Instance().Flush();

        std::ifstream file(testLogPath);
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        REQUIRE(content.find("Debug via macro") != std::string::npos);
        REQUIRE(content.find("Info via macro") != std::string::npos);
        REQUIRE(content.find("Warning via macro") != std::string::npos);
        REQUIRE(content.find("Error via macro") != std::string::npos);
    }

    Logger::Instance().Shutdown();

    // Clean up
    if (std::filesystem::exists(testLogPath)) {
        std::filesystem::remove(testLogPath);
    }
}

TEST_CASE("Logger format strings", "[logger]") {
    std::filesystem::path testLogPath = std::filesystem::temp_directory_path() / "metaimgui_test_format.log";

    // Clean up
    if (std::filesystem::exists(testLogPath)) {
        std::filesystem::remove(testLogPath);
    }

    Logger::Instance().Initialize(testLogPath, LogLevel::Debug);

    SECTION("Simple format strings work") {
        Logger::Instance().Info("Value: {}", 42);
        Logger::Instance().Info("String: {}", "test");
        Logger::Instance().Info("Multiple: {} and {}", 1, 2);

        Logger::Instance().Flush();

        std::ifstream file(testLogPath);
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        REQUIRE(content.find("Value: 42") != std::string::npos);
        REQUIRE(content.find("String: test") != std::string::npos);
        REQUIRE(content.find("Multiple: 1 and 2") != std::string::npos);
    }

    Logger::Instance().Shutdown();

    // Clean up
    if (std::filesystem::exists(testLogPath)) {
        std::filesystem::remove(testLogPath);
    }
}

TEST_CASE("Logger thread safety", "[logger]") {
    std::filesystem::path testLogPath = std::filesystem::temp_directory_path() / "metaimgui_test_threads.log";

    // Clean up
    if (std::filesystem::exists(testLogPath)) {
        std::filesystem::remove(testLogPath);
    }

    Logger::Instance().Initialize(testLogPath, LogLevel::Debug);

    SECTION("Concurrent logging from multiple threads") {
        const int numThreads = 4;
        const int messagesPerThread = 10;

        std::vector<std::thread> threads;
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back([i, messagesPerThread]() {
                for (int j = 0; j < messagesPerThread; ++j) {
                    Logger::Instance().Info("Thread {} Message {}", i, j);
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        Logger::Instance().Flush();

        // Count messages in log file
        std::ifstream file(testLogPath);
        std::string line;
        int messageCount = 0;
        while (std::getline(file, line)) {
            if (line.find("Thread") != std::string::npos && line.find("Message") != std::string::npos) {
                messageCount++;
            }
        }

        REQUIRE(messageCount == numThreads * messagesPerThread);
    }

    Logger::Instance().Shutdown();

    // Clean up
    if (std::filesystem::exists(testLogPath)) {
        std::filesystem::remove(testLogPath);
    }
}

