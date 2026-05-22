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

#pragma once

#include <atomic>
#include <filesystem>
#include <format>
#include <memory>
#include <string_view>
#include <utility>

// Heavy I/O and synchronisation headers (<fstream>, <sstream>, <mutex>) live
// behind the pimpl in the .cpp so they don't leak into every TU that logs.
// <filesystem> stays here because std::filesystem::path is part of the public API
// and isn't reliably forward-declarable across libstdc++/libc++/MSVC STL.

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
 * @brief Thread-safe logger with file and console output.
 *
 * Templated entry points format with std::vformat (C++20). The minimum-level
 * filter is checked through an atomic so the early-out path is lock-free.
 *
 * Usage:
 * @code
 * LOG_INFO("Application started");
 * LOG_ERROR("Failed to load file: {}", filename);
 * @endcode
 */
class Logger {
public:
    static Logger& Instance();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    void Initialize(const std::filesystem::path& logFilePath, LogLevel minLevel = LogLevel::Info);
    void Shutdown();

    void SetLevel(LogLevel level) noexcept {
        m_minLevel.store(level, std::memory_order_release);
    }

    [[nodiscard]] LogLevel GetLevel() const noexcept {
        return m_minLevel.load(std::memory_order_acquire);
    }

    void SetConsoleOutput(bool enable);
    void SetFileOutput(bool enable);
    void Flush();

    [[nodiscard]] std::filesystem::path GetLogFilePath() const;

    template <typename... Args>
    void Debug(std::format_string<Args...> fmt, Args&&... args) {
        if (LogLevel::Debug >= GetLevel()) {
            LogVFormat(LogLevel::Debug, fmt.get(), std::make_format_args(args...));
        }
    }

    template <typename... Args>
    void Info(std::format_string<Args...> fmt, Args&&... args) {
        if (LogLevel::Info >= GetLevel()) {
            LogVFormat(LogLevel::Info, fmt.get(), std::make_format_args(args...));
        }
    }

    template <typename... Args>
    void Warning(std::format_string<Args...> fmt, Args&&... args) {
        if (LogLevel::Warning >= GetLevel()) {
            LogVFormat(LogLevel::Warning, fmt.get(), std::make_format_args(args...));
        }
    }

    template <typename... Args>
    void Error(std::format_string<Args...> fmt, Args&&... args) {
        if (LogLevel::Error >= GetLevel()) {
            LogVFormat(LogLevel::Error, fmt.get(), std::make_format_args(args...));
        }
    }

    template <typename... Args>
    void Fatal(std::format_string<Args...> fmt, Args&&... args) {
        if (LogLevel::Fatal >= GetLevel()) {
            LogVFormat(LogLevel::Fatal, fmt.get(), std::make_format_args(args...));
        }
    }

    // Overloads for plain string messages — skip vformat entirely.
    void Debug(std::string_view message) {
        if (LogLevel::Debug >= GetLevel()) {
            LogPlain(LogLevel::Debug, message);
        }
    }
    void Info(std::string_view message) {
        if (LogLevel::Info >= GetLevel()) {
            LogPlain(LogLevel::Info, message);
        }
    }
    void Warning(std::string_view message) {
        if (LogLevel::Warning >= GetLevel()) {
            LogPlain(LogLevel::Warning, message);
        }
    }
    void Error(std::string_view message) {
        if (LogLevel::Error >= GetLevel()) {
            LogPlain(LogLevel::Error, message);
        }
    }
    void Fatal(std::string_view message) {
        if (LogLevel::Fatal >= GetLevel()) {
            LogPlain(LogLevel::Fatal, message);
        }
    }

private:
    Logger();
    ~Logger();

    // Pimpl holds <fstream>/<filesystem>/<mutex> so they don't leak into TUs.
    // The destructor is defined in the .cpp so unique_ptr<Impl> can hold an
    // incomplete type here.
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    // Hot-path: read by every Debug/Info/... template before any locking.
    std::atomic<LogLevel> m_minLevel{LogLevel::Info};

    void LogVFormat(LogLevel level, std::string_view fmt, std::format_args args);
    void LogPlain(LogLevel level, std::string_view message);
};

} // namespace MetaImGUI

// Convenience macros
#define LOG_DEBUG(...) MetaImGUI::Logger::Instance().Debug(__VA_ARGS__)
#define LOG_INFO(...) MetaImGUI::Logger::Instance().Info(__VA_ARGS__)
#define LOG_WARNING(...) MetaImGUI::Logger::Instance().Warning(__VA_ARGS__)
#define LOG_ERROR(...) MetaImGUI::Logger::Instance().Error(__VA_ARGS__)
#define LOG_FATAL(...) MetaImGUI::Logger::Instance().Fatal(__VA_ARGS__)
