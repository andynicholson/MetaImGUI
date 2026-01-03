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
#include <iomanip>
#include <iostream>

namespace MetaImGUI {

Logger::Logger() = default;

Logger::~Logger() {
    Shutdown();
}

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

void Logger::Initialize(const std::filesystem::path& logFilePath, LogLevel minLevel) {
    const std::lock_guard<std::mutex> lock(m_mutex);

    m_minLevel = minLevel;
    m_logFilePath = logFilePath;

    if (!logFilePath.empty()) {
        // Ensure parent directory exists
        auto parentPath = logFilePath.parent_path();
        if (!parentPath.empty() && !std::filesystem::exists(parentPath)) {
            std::error_code ec;
            std::filesystem::create_directories(parentPath, ec);
            if (ec) {
                std::cerr << "Warning: Could not create log directory: " << ec.message() << '\n';
            }
        }

        m_logFile.open(logFilePath, std::ios::out | std::ios::app);
        if (m_logFile.is_open()) {
            m_fileOutput = true;
            m_logFile << "\n========== Log Session Started: " << GetTimestamp() << " ==========\n";
            m_logFile.flush();
        } else {
            std::cerr << "Failed to open log file: " << logFilePath << '\n';
            m_fileOutput = false;
        }
    }

    if (m_consoleOutput) {
        std::cout << "Logger initialized (Level: " << LevelToString(minLevel) << ")" << '\n';
    }
}

void Logger::Shutdown() {
    const std::lock_guard<std::mutex> lock(m_mutex);

    if (m_logFile.is_open()) {
        m_logFile << "========== Log Session Ended: " << GetTimestamp() << " ==========\n\n";
        m_logFile.close();
    }
}

void Logger::SetLevel(LogLevel level) {
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_minLevel = level;
}

LogLevel Logger::GetLevel() const {
    const std::lock_guard<std::mutex> lock(m_mutex);
    return m_minLevel;
}

void Logger::SetConsoleOutput(bool enable) {
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_consoleOutput = enable;
}

void Logger::SetFileOutput(bool enable) {
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_fileOutput = enable && m_logFile.is_open();
}

void Logger::Flush() {
    const std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile.is_open()) {
        m_logFile.flush();
    }
    std::cout.flush();
    std::cerr.flush();
}

std::filesystem::path Logger::GetLogFilePath() const {
    const std::lock_guard<std::mutex> lock(m_mutex);
    return m_logFilePath;
}

void Logger::LogMessage(LogLevel level, const std::string& message) {
    const std::lock_guard<std::mutex> lock(m_mutex);

    const std::string timestamp = GetTimestamp();
    const std::string levelStr = LevelToString(level);
    const std::string formattedMessage = "[" + timestamp + "] [" + levelStr + "] " + message;

    // Console output with colors
    if (m_consoleOutput) {
        const char* color = LevelToColor(level);
        const char* reset = "\033[0m";

        std::ostream& out = (level >= LogLevel::Error) ? std::cerr : std::cout;
        out << color << formattedMessage << reset << '\n';
    }

    // File output without colors
    if (m_fileOutput && m_logFile.is_open()) {
        m_logFile << formattedMessage << '\n';

        // Auto-flush for errors and above
        if (level >= LogLevel::Error) {
            m_logFile.flush();
        }
    }
}

std::string Logger::GetTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

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

std::string Logger::LevelToString(LogLevel level) const {
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
        default:
            return "?????";
    }
}

const char* Logger::LevelToColor(LogLevel level) const {
    switch (level) {
        case LogLevel::Debug:
            return "\033[36m"; // Cyan
        case LogLevel::Info:
            return "\033[32m"; // Green
        case LogLevel::Warning:
            return "\033[33m"; // Yellow
        case LogLevel::Error:
            return "\033[31m"; // Red
        case LogLevel::Fatal:
            return "\033[35m"; // Magenta
        default:
            return "\033[0m"; // Reset
    }
}

} // namespace MetaImGUI
