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

#include "Logger.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

namespace MetaImGUI {

struct Logger::Impl {
    bool consoleOutput = true;
    bool fileOutput = false;
    std::filesystem::path logFilePath;
    std::ofstream logFile;
    mutable std::mutex mutex;
};

namespace {

std::string CurrentTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm_buf{};
#ifdef _WIN32
    localtime_s(&tm_buf, &time);
#else
    localtime_r(&time, &tm_buf);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string_view LevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO ";
        case LogLevel::Warning:
            return "WARN ";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Fatal:
            return "FATAL";
    }
    return "?????";
}

const char* LevelToColor(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "\033[36m";
        case LogLevel::Info:
            return "\033[32m";
        case LogLevel::Warning:
            return "\033[33m";
        case LogLevel::Error:
            return "\033[31m";
        case LogLevel::Fatal:
            return "\033[35m";
    }
    return "\033[0m";
}

} // namespace

Logger::Logger() : m_impl(std::make_unique<Impl>()) {}

Logger::~Logger() {
    Shutdown();
}

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

void Logger::Initialize(const std::filesystem::path& logFilePath, LogLevel minLevel) {
    const std::lock_guard<std::mutex> lock(m_impl->mutex);

    m_minLevel.store(minLevel, std::memory_order_release);
    m_impl->logFilePath = logFilePath;

    if (!logFilePath.empty()) {
        const auto parentPath = logFilePath.parent_path();
        if (!parentPath.empty() && !std::filesystem::exists(parentPath)) {
            std::error_code ec;
            std::filesystem::create_directories(parentPath, ec);
            if (ec) {
                std::cerr << "Warning: Could not create log directory: " << ec.message() << '\n';
            }
        }

        m_impl->logFile.open(logFilePath, std::ios::out | std::ios::app);
        if (m_impl->logFile.is_open()) {
            m_impl->fileOutput = true;
            m_impl->logFile << "\n========== Log Session Started: " << CurrentTimestamp() << " ==========\n";
            m_impl->logFile.flush();
        } else {
            std::cerr << "Failed to open log file: " << logFilePath << '\n';
            m_impl->fileOutput = false;
        }
    }

    if (m_impl->consoleOutput) {
        std::cout << "Logger initialized (Level: " << LevelToString(minLevel) << ")" << '\n';
    }
}

void Logger::Shutdown() {
    const std::lock_guard<std::mutex> lock(m_impl->mutex);

    if (m_impl->logFile.is_open()) {
        m_impl->logFile << "========== Log Session Ended: " << CurrentTimestamp() << " ==========\n\n";
        m_impl->logFile.close();
    }
}

void Logger::SetConsoleOutput(bool enable) {
    const std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->consoleOutput = enable;
}

void Logger::SetFileOutput(bool enable) {
    const std::lock_guard<std::mutex> lock(m_impl->mutex);
    m_impl->fileOutput = enable && m_impl->logFile.is_open();
}

void Logger::Flush() {
    const std::lock_guard<std::mutex> lock(m_impl->mutex);
    if (m_impl->logFile.is_open()) {
        m_impl->logFile.flush();
    }
    std::cout.flush();
    std::cerr.flush();
}

std::filesystem::path Logger::GetLogFilePath() const {
    const std::lock_guard<std::mutex> lock(m_impl->mutex);
    return m_impl->logFilePath;
}

void Logger::LogVFormat(LogLevel level, std::string_view fmt, std::format_args args) {
    std::string message;
    try {
        message = std::vformat(fmt, args);
    } catch (const std::format_error& e) {
        // Fall back to the raw format string so a bad spec never silently drops
        // the message — the surrounding text still helps locate the call site.
        message = std::string(fmt) + " [format_error: " + e.what() + "]";
    }
    LogPlain(level, message);
}

void Logger::LogPlain(LogLevel level, std::string_view message) {
    // Format the line *outside* the mutex so a logging call from inside an
    // ostream/streambuf during the write step can never re-enter and deadlock.
    const std::string timestamp = CurrentTimestamp();
    const std::string_view levelStr = LevelToString(level);

    std::string formatted;
    formatted.reserve(timestamp.size() + levelStr.size() + message.size() + 8);
    formatted.append("[").append(timestamp).append("] [").append(levelStr).append("] ").append(message);

    bool consoleOutput = false;
    bool fileOutput = false;
    {
        const std::lock_guard<std::mutex> lock(m_impl->mutex);
        consoleOutput = m_impl->consoleOutput;
        fileOutput = m_impl->fileOutput && m_impl->logFile.is_open();

        if (fileOutput) {
            m_impl->logFile << formatted << '\n';
            if (level >= LogLevel::Error) {
                m_impl->logFile.flush();
            }
        }
    }

    // Console writes happen unlocked: cout/cerr have their own internal sync.
    if (consoleOutput) {
        const char* color = LevelToColor(level);
        const char* reset = "\033[0m";
        std::ostream& out = (level >= LogLevel::Error) ? std::cerr : std::cout;
        out << color << formatted << reset << '\n';
    }
}

} // namespace MetaImGUI
