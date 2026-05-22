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

#include "ConfigManager.h"

#include "Logger.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <mutex>
#include <shared_mutex>

#ifdef _WIN32
#include <shlobj.h>
#include <windows.h>
#else
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

using json = nlohmann::json;

namespace MetaImGUI {

// Pimpl implementation hides the JSON dependency and the synchronisation
// primitive from every translation unit that includes ConfigManager.h.
//
// shared_mutex chosen over plain mutex because reads (Get*) far outnumber
// writes (Set*/Load/Save/Reset) at runtime — most frames only read.
struct ConfigManager::Impl {
    json config;
    std::filesystem::path configPath;
    size_t maxRecentFiles = 10;
    mutable std::shared_mutex mutex;

    // Default values
    static constexpr int DEFAULT_WINDOW_WIDTH = 1200;
    static constexpr int DEFAULT_WINDOW_HEIGHT = 800;
    static constexpr const char* DEFAULT_THEME = "Modern";

    // Reset the config to defaults assuming the caller already holds the
    // exclusive lock. Used by Reset() (which takes the lock) and by Load()
    // on parse failure (which already holds it).
    void ResetUnlocked() {
        config = json::object();
        config["window"]["width"] = DEFAULT_WINDOW_WIDTH;
        config["window"]["height"] = DEFAULT_WINDOW_HEIGHT;
        config["window"]["maximized"] = false;
        config["theme"] = DEFAULT_THEME;
        config["recentFiles"] = json::array();
        config["settings"] = json::object();
    }
};

ConfigManager::ConfigManager() : m_impl(std::make_unique<Impl>()) {
    m_impl->configPath = GetConfigPath();
    // No lock needed: the object isn't visible to any other thread yet.
    m_impl->ResetUnlocked();
}

// Out-of-line so unique_ptr<Impl> can hold an incomplete type in the header.
ConfigManager::~ConfigManager() = default;

bool ConfigManager::Load() {
    const std::unique_lock lock(m_impl->mutex);
    try {
        if (!std::filesystem::exists(m_impl->configPath)) {
            LOG_INFO("Config file not found, using defaults");
            return false;
        }

        std::ifstream file(m_impl->configPath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open config file: {}", m_impl->configPath.string());
            return false;
        }

        m_impl->config = json::parse(file);
        LOG_INFO("Configuration loaded from: {}", m_impl->configPath.string());
        return true;
    } catch (const json::exception& e) {
        LOG_ERROR("Failed to parse config file: {}", e.what());
        m_impl->ResetUnlocked(); // Already hold the exclusive lock.
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load config: {}", e.what());
        return false;
    }
}

bool ConfigManager::Save() {
    // Snapshot under shared lock so concurrent readers aren't blocked on disk I/O.
    json snapshot;
    std::filesystem::path path;
    {
        const std::shared_lock lock(m_impl->mutex);
        snapshot = m_impl->config;
        path = m_impl->configPath;
    }

    try {
        if (!EnsureConfigDirectoryExists()) {
            LOG_ERROR("Failed to create config directory");
            return false;
        }

        std::ofstream file(path);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open config file for writing: {}", path.string());
            return false;
        }

        file << snapshot.dump(2);
        LOG_INFO("Configuration saved to: {}", path.string());
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to save config: {}", e.what());
        return false;
    }
}

void ConfigManager::Reset() {
    const std::unique_lock lock(m_impl->mutex);
    m_impl->ResetUnlocked();
}

bool ConfigManager::ConfigFileExists() const {
    const std::shared_lock lock(m_impl->mutex);
    return std::filesystem::exists(m_impl->configPath);
}

std::filesystem::path ConfigManager::GetConfigPath() const {
    return GetConfigDirectory() / "config.json";
}

// Window settings

void ConfigManager::SetWindowPosition(int x, int y) {
    const std::unique_lock lock(m_impl->mutex);
    m_impl->config["window"]["x"] = x;
    m_impl->config["window"]["y"] = y;
}

void ConfigManager::SetWindowSize(int width, int height) {
    const std::unique_lock lock(m_impl->mutex);
    m_impl->config["window"]["width"] = width;
    m_impl->config["window"]["height"] = height;
}

std::optional<std::pair<int, int>> ConfigManager::GetWindowPosition() const {
    const std::shared_lock lock(m_impl->mutex);
    try {
        if (m_impl->config.contains("window") && m_impl->config["window"].contains("x") &&
            m_impl->config["window"].contains("y")) {
            return std::make_pair(m_impl->config["window"]["x"].get<int>(), m_impl->config["window"]["y"].get<int>());
        }
    } catch (const json::exception& e) {
        LOG_WARNING("Failed to get window position from config: {}", e.what());
    }
    return std::nullopt;
}

std::optional<std::pair<int, int>> ConfigManager::GetWindowSize() const {
    const std::shared_lock lock(m_impl->mutex);
    try {
        if (m_impl->config.contains("window") && m_impl->config["window"].contains("width") &&
            m_impl->config["window"].contains("height")) {
            return std::make_pair(m_impl->config["window"]["width"].get<int>(),
                                  m_impl->config["window"]["height"].get<int>());
        }
    } catch (const json::exception& e) {
        LOG_WARNING("Failed to get window size from config: {}", e.what());
    }
    return std::nullopt;
}

void ConfigManager::SetWindowMaximized(bool maximized) {
    const std::unique_lock lock(m_impl->mutex);
    m_impl->config["window"]["maximized"] = maximized;
}

bool ConfigManager::GetWindowMaximized() const {
    const std::shared_lock lock(m_impl->mutex);
    try {
        if (m_impl->config.contains("window") && m_impl->config["window"].contains("maximized")) {
            return m_impl->config["window"]["maximized"].get<bool>();
        }
    } catch (const json::exception& e) {
        LOG_WARNING("Failed to get window maximized state from config: {}", e.what());
    }
    return false;
}

// Theme settings

void ConfigManager::SetTheme(const std::string& theme) {
    const std::unique_lock lock(m_impl->mutex);
    m_impl->config["theme"] = theme;
}

std::string ConfigManager::GetTheme() const {
    const std::shared_lock lock(m_impl->mutex);
    try {
        if (m_impl->config.contains("theme")) {
            return m_impl->config["theme"].get<std::string>();
        }
    } catch (const json::exception& e) {
        LOG_WARNING("Failed to get theme from config: {}", e.what());
    }
    return Impl::DEFAULT_THEME;
}

// Recent files

void ConfigManager::AddRecentFile(const std::string& filepath) {
    const std::unique_lock lock(m_impl->mutex);
    if (!m_impl->config.contains("recentFiles")) {
        m_impl->config["recentFiles"] = json::array();
    }

    auto& recentFiles = m_impl->config["recentFiles"];

    // Remove if already exists (to move to front)
    for (auto it = recentFiles.begin(); it != recentFiles.end(); ++it) {
        if (*it == filepath) {
            recentFiles.erase(it);
            break;
        }
    }

    // Add to front
    recentFiles.insert(recentFiles.begin(), filepath);

    // Limit size
    while (recentFiles.size() > m_impl->maxRecentFiles) {
        recentFiles.erase(recentFiles.end() - 1);
    }
}

std::vector<std::string> ConfigManager::GetRecentFiles() const {
    const std::shared_lock lock(m_impl->mutex);
    std::vector<std::string> result;
    try {
        if (m_impl->config.contains("recentFiles")) {
            for (const auto& file : m_impl->config["recentFiles"]) {
                result.push_back(file.get<std::string>());
            }
        }
    } catch (const json::exception& e) {
        LOG_WARNING("Failed to get recent files from config: {}", e.what());
    }
    return result;
}

void ConfigManager::ClearRecentFiles() {
    const std::unique_lock lock(m_impl->mutex);
    m_impl->config["recentFiles"] = json::array();
}

void ConfigManager::SetMaxRecentFiles(size_t max) {
    const std::unique_lock lock(m_impl->mutex);
    m_impl->maxRecentFiles = max;
}

// Generic settings

void ConfigManager::SetString(const std::string& key, const std::string& value) {
    const std::unique_lock lock(m_impl->mutex);
    if (!m_impl->config.contains("settings")) {
        m_impl->config["settings"] = json::object();
    }
    m_impl->config["settings"][key] = value;
}

std::optional<std::string> ConfigManager::GetString(const std::string& key) const {
    const std::shared_lock lock(m_impl->mutex);
    try {
        if (m_impl->config.contains("settings") && m_impl->config["settings"].contains(key)) {
            return m_impl->config["settings"][key].get<std::string>();
        }
    } catch (const json::exception& e) {
        LOG_WARNING("Failed to get string '{}' from config: {}", key, e.what());
    }
    return std::nullopt;
}

void ConfigManager::SetInt(const std::string& key, int value) {
    const std::unique_lock lock(m_impl->mutex);
    if (!m_impl->config.contains("settings")) {
        m_impl->config["settings"] = json::object();
    }
    m_impl->config["settings"][key] = value;
}

std::optional<int> ConfigManager::GetInt(const std::string& key) const {
    const std::shared_lock lock(m_impl->mutex);
    try {
        if (m_impl->config.contains("settings") && m_impl->config["settings"].contains(key)) {
            return m_impl->config["settings"][key].get<int>();
        }
    } catch (const json::exception& e) {
        LOG_WARNING("Failed to get int '{}' from config: {}", key, e.what());
    }
    return std::nullopt;
}

void ConfigManager::SetBool(const std::string& key, bool value) {
    const std::unique_lock lock(m_impl->mutex);
    if (!m_impl->config.contains("settings")) {
        m_impl->config["settings"] = json::object();
    }
    m_impl->config["settings"][key] = value;
}

std::optional<bool> ConfigManager::GetBool(const std::string& key) const {
    const std::shared_lock lock(m_impl->mutex);
    try {
        if (m_impl->config.contains("settings") && m_impl->config["settings"].contains(key)) {
            return m_impl->config["settings"][key].get<bool>();
        }
    } catch (const json::exception& e) {
        LOG_WARNING("Failed to get bool '{}' from config: {}", key, e.what());
    }
    return std::nullopt;
}

void ConfigManager::SetFloat(const std::string& key, float value) {
    const std::unique_lock lock(m_impl->mutex);
    if (!m_impl->config.contains("settings")) {
        m_impl->config["settings"] = json::object();
    }
    m_impl->config["settings"][key] = value;
}

std::optional<float> ConfigManager::GetFloat(const std::string& key) const {
    const std::shared_lock lock(m_impl->mutex);
    try {
        if (m_impl->config.contains("settings") && m_impl->config["settings"].contains(key)) {
            return m_impl->config["settings"][key].get<float>();
        }
    } catch (const json::exception& e) {
        LOG_WARNING("Failed to get float '{}' from config: {}", key, e.what());
    }
    return std::nullopt;
}

bool ConfigManager::HasKey(const std::string& key) const {
    const std::shared_lock lock(m_impl->mutex);
    return m_impl->config.contains("settings") && m_impl->config["settings"].contains(key);
}

void ConfigManager::RemoveKey(const std::string& key) {
    const std::unique_lock lock(m_impl->mutex);
    if (m_impl->config.contains("settings")) {
        m_impl->config["settings"].erase(key);
    }
}

std::vector<std::string> ConfigManager::GetAllKeys() const {
    const std::shared_lock lock(m_impl->mutex);
    std::vector<std::string> keys;
    try {
        if (m_impl->config.contains("settings")) {
            for (auto it = m_impl->config["settings"].begin(); it != m_impl->config["settings"].end(); ++it) {
                keys.push_back(it.key());
            }
        }
    } catch (const json::exception& e) {
        LOG_WARNING("Failed to get all keys from config: {}", e.what());
    }
    return keys;
}

// Platform-specific helpers

std::filesystem::path ConfigManager::GetConfigDirectory() {
#ifdef _WIN32
    // Windows: %APPDATA%/MetaImGUI
    // Use SHGetKnownFolderPath for long path support (not limited to MAX_PATH)
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path))) {
        std::filesystem::path result(path);
        CoTaskMemFree(path); // Must free the allocated string
        return result / "MetaImGUI";
    }
    return std::filesystem::path("./config");
#elif defined(__APPLE__)
    // macOS: ~/Library/Application Support/MetaImGUI
    // NOLINTNEXTLINE(concurrency-mt-unsafe) - Safe: called during single-threaded initialization
    const char* home = getenv("HOME");
    if (home) {
        return std::filesystem::path(home) / "Library" / "Application Support" / "MetaImGUI";
    }
    return std::filesystem::path("./config");
#else
    // Linux: ~/.config/MetaImGUI
    // NOLINTNEXTLINE(concurrency-mt-unsafe) - Safe: called during single-threaded initialization
    const char* xdgConfig = getenv("XDG_CONFIG_HOME");
    if (xdgConfig != nullptr) {
        return std::filesystem::path(xdgConfig) / "MetaImGUI";
    }

    // NOLINTNEXTLINE(concurrency-mt-unsafe) - Safe: called during single-threaded initialization
    const char* home = getenv("HOME");
    if (home != nullptr) {
        return std::filesystem::path(home) / ".config" / "MetaImGUI";
    }

    return {"./config"};
#endif
}

bool ConfigManager::EnsureConfigDirectoryExists() {
    try {
        const std::filesystem::path dir = GetConfigDirectory();
        if (!std::filesystem::exists(dir)) {
            return std::filesystem::create_directories(dir);
        }
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR("Failed to create config directory: {}", e.what());
        return false;
    }
}

} // namespace MetaImGUI
