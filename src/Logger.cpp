#include "Logger.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

namespace MetaImGUI {

Logger::Logger()
    : m_minLevel(LogLevel::Info), m_consoleOutput(true), m_fileOutput(false) {}

Logger::~Logger() {
    Shutdown();
}

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

void Logger::Initialize(const std::filesystem::path& logFilePath, LogLevel minLevel) {
    std::lock_guard<std::mutex> lock(m_mutex);

    m_minLevel = minLevel;
    m_logFilePath = logFilePath;

    if (!logFilePath.empty()) {
        // Ensure parent directory exists
        auto parentPath = logFilePath.parent_path();
        if (!parentPath.empty() && !std::filesystem::exists(parentPath)) {
            std::filesystem::create_directories(parentPath);
        }

        m_logFile.open(logFilePath, std::ios::out | std::ios::app);
        if (m_logFile.is_open()) {
            m_fileOutput = true;
            m_logFile << "\n========== Log Session Started: " << GetTimestamp() << " ==========\n";
            m_logFile.flush();
        } else {
            std::cerr << "Failed to open log file: " << logFilePath << std::endl;
            m_fileOutput = false;
        }
    }

    if (m_consoleOutput) {
        std::cout << "Logger initialized (Level: " << LevelToString(minLevel) << ")" << std::endl;
    }
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_logFile.is_open()) {
        m_logFile << "========== Log Session Ended: " << GetTimestamp() << " ==========\n\n";
        m_logFile.close();
    }
}

void Logger::SetLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minLevel = level;
}

LogLevel Logger::GetLevel() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_minLevel;
}

void Logger::SetConsoleOutput(bool enable) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_consoleOutput = enable;
}

void Logger::SetFileOutput(bool enable) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_fileOutput = enable && m_logFile.is_open();
}

void Logger::Flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile.is_open()) {
        m_logFile.flush();
    }
    std::cout.flush();
    std::cerr.flush();
}

std::filesystem::path Logger::GetLogFilePath() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_logFilePath;
}

void Logger::LogMessage(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string timestamp = GetTimestamp();
    std::string levelStr = LevelToString(level);
    std::string formattedMessage = "[" + timestamp + "] [" + levelStr + "] " + message;

    // Console output with colors
    if (m_consoleOutput) {
        const char* color = LevelToColor(level);
        const char* reset = "\033[0m";

        std::ostream& out = (level >= LogLevel::Error) ? std::cerr : std::cout;
        out << color << formattedMessage << reset << std::endl;
    }

    // File output without colors
    if (m_fileOutput && m_logFile.is_open()) {
        m_logFile << formattedMessage << std::endl;

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

    std::tm tm_buf;
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

