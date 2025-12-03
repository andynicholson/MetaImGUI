#include "ConfigManager.h"

#include "Logger.h"

#include <nlohmann/json.hpp>

#include <fstream>

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

// Pimpl implementation to hide JSON dependency from header
struct ConfigManager::Impl {
    json config;
    std::filesystem::path configPath;
    size_t maxRecentFiles = 10;

    // Default values
    static constexpr int DEFAULT_WINDOW_WIDTH = 1200;
    static constexpr int DEFAULT_WINDOW_HEIGHT = 800;
    static constexpr const char* DEFAULT_THEME = "Modern";
};

ConfigManager::ConfigManager() : m_impl(std::make_unique<Impl>()) {
    m_impl->configPath = GetConfigPath();
    Reset(); // Initialize with defaults
}

ConfigManager::~ConfigManager() = default;

bool ConfigManager::Load() {
    try {
        if (!ConfigFileExists()) {
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
        Reset(); // Reset to defaults on parse error
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load config: {}", e.what());
        return false;
    }
}

bool ConfigManager::Save() {
    try {
        if (!EnsureConfigDirectoryExists()) {
            LOG_ERROR("Failed to create config directory");
            return false;
        }

        std::ofstream file(m_impl->configPath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open config file for writing: {}", m_impl->configPath.string());
            return false;
        }

        file << m_impl->config.dump(2); // Pretty print with 2-space indent
        LOG_INFO("Configuration saved to: {}", m_impl->configPath.string());
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to save config: {}", e.what());
        return false;
    }
}

void ConfigManager::Reset() {
    m_impl->config = json::object();

    // Set default values
    m_impl->config["window"]["width"] = Impl::DEFAULT_WINDOW_WIDTH;
    m_impl->config["window"]["height"] = Impl::DEFAULT_WINDOW_HEIGHT;
    m_impl->config["window"]["maximized"] = false;
    m_impl->config["theme"] = Impl::DEFAULT_THEME;
    m_impl->config["recentFiles"] = json::array();
    m_impl->config["settings"] = json::object();
}

bool ConfigManager::ConfigFileExists() const {
    return std::filesystem::exists(m_impl->configPath);
}

std::filesystem::path ConfigManager::GetConfigPath() const {
    return GetConfigDirectory() / "config.json";
}

// Window settings

void ConfigManager::SetWindowPosition(int x, int y) {
    m_impl->config["window"]["x"] = x;
    m_impl->config["window"]["y"] = y;
}

void ConfigManager::SetWindowSize(int width, int height) {
    m_impl->config["window"]["width"] = width;
    m_impl->config["window"]["height"] = height;
}

std::optional<std::pair<int, int>> ConfigManager::GetWindowPosition() const {
    try {
        if (m_impl->config.contains("window") && m_impl->config["window"].contains("x") &&
            m_impl->config["window"].contains("y")) {
            return std::make_pair(m_impl->config["window"]["x"].get<int>(), m_impl->config["window"]["y"].get<int>());
        }
    } catch (const json::exception&) {
    }
    return std::nullopt;
}

std::optional<std::pair<int, int>> ConfigManager::GetWindowSize() const {
    try {
        if (m_impl->config.contains("window") && m_impl->config["window"].contains("width") &&
            m_impl->config["window"].contains("height")) {
            return std::make_pair(m_impl->config["window"]["width"].get<int>(),
                                  m_impl->config["window"]["height"].get<int>());
        }
    } catch (const json::exception&) {
    }
    return std::nullopt;
}

void ConfigManager::SetWindowMaximized(bool maximized) {
    m_impl->config["window"]["maximized"] = maximized;
}

bool ConfigManager::GetWindowMaximized() const {
    try {
        if (m_impl->config.contains("window") && m_impl->config["window"].contains("maximized")) {
            return m_impl->config["window"]["maximized"].get<bool>();
        }
    } catch (const json::exception&) {
    }
    return false;
}

// Theme settings

void ConfigManager::SetTheme(const std::string& theme) {
    m_impl->config["theme"] = theme;
}

std::string ConfigManager::GetTheme() const {
    try {
        if (m_impl->config.contains("theme")) {
            return m_impl->config["theme"].get<std::string>();
        }
    } catch (const json::exception&) {
    }
    return Impl::DEFAULT_THEME;
}

// Recent files

void ConfigManager::AddRecentFile(const std::string& filepath) {
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
    std::vector<std::string> result;
    try {
        if (m_impl->config.contains("recentFiles")) {
            for (const auto& file : m_impl->config["recentFiles"]) {
                result.push_back(file.get<std::string>());
            }
        }
    } catch (const json::exception&) {
    }
    return result;
}

void ConfigManager::ClearRecentFiles() {
    m_impl->config["recentFiles"] = json::array();
}

void ConfigManager::SetMaxRecentFiles(size_t max) {
    m_impl->maxRecentFiles = max;
}

// Generic settings

void ConfigManager::SetString(const std::string& key, const std::string& value) {
    if (!m_impl->config.contains("settings")) {
        m_impl->config["settings"] = json::object();
    }
    m_impl->config["settings"][key] = value;
}

std::optional<std::string> ConfigManager::GetString(const std::string& key) const {
    try {
        if (m_impl->config.contains("settings") && m_impl->config["settings"].contains(key)) {
            return m_impl->config["settings"][key].get<std::string>();
        }
    } catch (const json::exception&) {
    }
    return std::nullopt;
}

void ConfigManager::SetInt(const std::string& key, int value) {
    if (!m_impl->config.contains("settings")) {
        m_impl->config["settings"] = json::object();
    }
    m_impl->config["settings"][key] = value;
}

std::optional<int> ConfigManager::GetInt(const std::string& key) const {
    try {
        if (m_impl->config.contains("settings") && m_impl->config["settings"].contains(key)) {
            return m_impl->config["settings"][key].get<int>();
        }
    } catch (const json::exception&) {
    }
    return std::nullopt;
}

void ConfigManager::SetBool(const std::string& key, bool value) {
    if (!m_impl->config.contains("settings")) {
        m_impl->config["settings"] = json::object();
    }
    m_impl->config["settings"][key] = value;
}

std::optional<bool> ConfigManager::GetBool(const std::string& key) const {
    try {
        if (m_impl->config.contains("settings") && m_impl->config["settings"].contains(key)) {
            return m_impl->config["settings"][key].get<bool>();
        }
    } catch (const json::exception&) {
    }
    return std::nullopt;
}

void ConfigManager::SetFloat(const std::string& key, float value) {
    if (!m_impl->config.contains("settings")) {
        m_impl->config["settings"] = json::object();
    }
    m_impl->config["settings"][key] = value;
}

std::optional<float> ConfigManager::GetFloat(const std::string& key) const {
    try {
        if (m_impl->config.contains("settings") && m_impl->config["settings"].contains(key)) {
            return m_impl->config["settings"][key].get<float>();
        }
    } catch (const json::exception&) {
    }
    return std::nullopt;
}

bool ConfigManager::HasKey(const std::string& key) const {
    return m_impl->config.contains("settings") && m_impl->config["settings"].contains(key);
}

void ConfigManager::RemoveKey(const std::string& key) {
    if (m_impl->config.contains("settings")) {
        m_impl->config["settings"].erase(key);
    }
}

std::vector<std::string> ConfigManager::GetAllKeys() const {
    std::vector<std::string> keys;
    try {
        if (m_impl->config.contains("settings")) {
            for (auto it = m_impl->config["settings"].begin(); it != m_impl->config["settings"].end(); ++it) {
                keys.push_back(it.key());
            }
        }
    } catch (const json::exception&) {
    }
    return keys;
}

// Platform-specific helpers

std::filesystem::path ConfigManager::GetConfigDirectory() {
#ifdef _WIN32
    // Windows: %APPDATA%/MetaImGUI
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        return std::filesystem::path(path) / "MetaImGUI";
    }
    return std::filesystem::path("./config");
#elif defined(__APPLE__)
    // macOS: ~/Library/Application Support/MetaImGUI
    const char* home = getenv("HOME");
    if (home) {
        return std::filesystem::path(home) / "Library" / "Application Support" / "MetaImGUI";
    }
    return std::filesystem::path("./config");
#else
    // Linux: ~/.config/MetaImGUI
    const char* xdgConfig = getenv("XDG_CONFIG_HOME");
    if (xdgConfig) {
        return std::filesystem::path(xdgConfig) / "MetaImGUI";
    }

    const char* home = getenv("HOME");
    if (home) {
        return std::filesystem::path(home) / ".config" / "MetaImGUI";
    }

    return std::filesystem::path("./config");
#endif
}

bool ConfigManager::EnsureConfigDirectoryExists() {
    try {
        std::filesystem::path dir = GetConfigDirectory();
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
