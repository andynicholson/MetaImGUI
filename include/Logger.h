#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>

namespace MetaImGUI {

/**
 * @brief Log severity levels
 */
enum class LogLevel {
    Debug,   ///< Detailed debugging information
    Info,    ///< Informational messages
    Warning, ///< Warning messages
    Error,   ///< Error messages
    Fatal    ///< Fatal error messages
};

/**
 * @brief Simple logging system with file and console output
 *
 * Logger provides thread-safe logging with configurable severity levels,
 * timestamps, and output to both console and file.
 *
 * Usage:
 * @code
 * Logger::Instance().Info("Application started");
 * Logger::Instance().Error("Failed to load file: {}", filename);
 * LOG_DEBUG("Debug message");
 * LOG_INFO("Info message");
 * LOG_WARNING("Warning message");
 * LOG_ERROR("Error message");
 * @endcode
 */
class Logger {
public:
    /**
     * @brief Get singleton instance
     */
    static Logger& Instance();

    // Delete copy and move
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    /**
     * @brief Initialize logger with optional log file
     * @param logFilePath Path to log file (empty to disable file logging)
     * @param minLevel Minimum log level to output
     */
    void Initialize(const std::filesystem::path& logFilePath = "", LogLevel minLevel = LogLevel::Info);

    /**
     * @brief Shutdown logger and flush buffers
     */
    void Shutdown();

    /**
     * @brief Set minimum log level
     */
    void SetLevel(LogLevel level);

    /**
     * @brief Get current log level
     */
    LogLevel GetLevel() const;

    /**
     * @brief Enable/disable console output
     */
    void SetConsoleOutput(bool enable);

    /**
     * @brief Enable/disable file output
     */
    void SetFileOutput(bool enable);

    /**
     * @brief Flush log buffers
     */
    void Flush();

    /**
     * @brief Get log file path
     */
    std::filesystem::path GetLogFilePath() const;

    // Logging methods
    template <typename... Args>
    void Debug(std::string_view format, Args&&... args) {
        Log(LogLevel::Debug, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Info(std::string_view format, Args&&... args) {
        Log(LogLevel::Info, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Warning(std::string_view format, Args&&... args) {
        Log(LogLevel::Warning, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Error(std::string_view format, Args&&... args) {
        Log(LogLevel::Error, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Fatal(std::string_view format, Args&&... args) {
        Log(LogLevel::Fatal, format, std::forward<Args>(args)...);
    }

    // Simple overloads for no-argument messages
    void Debug(std::string_view message) {
        Log(LogLevel::Debug, message);
    }
    void Info(std::string_view message) {
        Log(LogLevel::Info, message);
    }
    void Warning(std::string_view message) {
        Log(LogLevel::Warning, message);
    }
    void Error(std::string_view message) {
        Log(LogLevel::Error, message);
    }
    void Fatal(std::string_view message) {
        Log(LogLevel::Fatal, message);
    }

private:
    Logger();
    ~Logger();

    template <typename... Args>
    void Log(LogLevel level, std::string_view format, Args&&... args) {
        if (level < m_minLevel) {
            return;
        }

        const std::string message = Format(format, std::forward<Args>(args)...);
        LogMessage(level, message);
    }

    void Log(LogLevel level, std::string_view message) {
        if (level < m_minLevel) {
            return;
        }
        LogMessage(level, std::string(message));
    }

    void LogMessage(LogLevel level, const std::string& message);

    // Simple format function (basic placeholder replacement)
    template <typename... Args>
    std::string Format(std::string_view format, Args&&... args) {
        std::ostringstream oss;
        FormatImpl(oss, format, std::forward<Args>(args)...);
        return oss.str();
    }

    template <typename T, typename... Args>
    void FormatImpl(std::ostringstream& oss, std::string_view format, T&& first, Args&&... rest) {
        const size_t pos = format.find("{}");
        if (pos != std::string_view::npos) {
            oss << format.substr(0, pos) << std::forward<T>(first);
            FormatImpl(oss, format.substr(pos + 2), std::forward<Args>(rest)...);
        } else {
            oss << format;
        }
    }

    void FormatImpl(std::ostringstream& oss, std::string_view format) {
        oss << format;
    }

    std::string GetTimestamp() const;
    std::string LevelToString(LogLevel level) const;
    const char* LevelToColor(LogLevel level) const;

    LogLevel m_minLevel = LogLevel::Info;
    bool m_consoleOutput = true;
    bool m_fileOutput = false;
    std::filesystem::path m_logFilePath;
    std::ofstream m_logFile;
    mutable std::mutex m_mutex;
};

} // namespace MetaImGUI

// Convenience macros
#define LOG_DEBUG(...) MetaImGUI::Logger::Instance().Debug(__VA_ARGS__)
#define LOG_INFO(...) MetaImGUI::Logger::Instance().Info(__VA_ARGS__)
#define LOG_WARNING(...) MetaImGUI::Logger::Instance().Warning(__VA_ARGS__)
#define LOG_ERROR(...) MetaImGUI::Logger::Instance().Error(__VA_ARGS__)
#define LOG_FATAL(...) MetaImGUI::Logger::Instance().Fatal(__VA_ARGS__)
