#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace MetaImGUI {

/**
 * @brief Configuration manager for persistent application settings
 *
 * ConfigManager handles loading, saving, and accessing application configuration
 * using JSON format. Settings are stored in the user's config directory.
 *
 * Supported platforms:
 * - Linux: ~/.config/MetaImGUI/
 * - Windows: %APPDATA%/MetaImGUI/
 * - macOS: ~/Library/Application Support/MetaImGUI/
 */
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    // Prevent copying but allow moving
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ConfigManager(ConfigManager&&) = default;
    ConfigManager& operator=(ConfigManager&&) = default;

    /**
     * @brief Load configuration from disk
     * @return true if configuration was loaded successfully
     */
    bool Load();

    /**
     * @brief Save configuration to disk
     * @return true if configuration was saved successfully
     */
    bool Save();

    /**
     * @brief Reset configuration to defaults
     */
    void Reset();

    /**
     * @brief Check if configuration file exists
     */
    bool ConfigFileExists() const;

    /**
     * @brief Get configuration file path
     */
    std::filesystem::path GetConfigPath() const;

    // Window settings
    void SetWindowPosition(int x, int y);
    void SetWindowSize(int width, int height);
    std::optional<std::pair<int, int>> GetWindowPosition() const;
    std::optional<std::pair<int, int>> GetWindowSize() const;
    void SetWindowMaximized(bool maximized);
    bool GetWindowMaximized() const;

    // Theme settings
    void SetTheme(const std::string& theme);
    std::string GetTheme() const;

    // Recent files
    void AddRecentFile(const std::string& filepath);
    std::vector<std::string> GetRecentFiles() const;
    void ClearRecentFiles();
    void SetMaxRecentFiles(size_t max);

    // Generic settings (string key-value pairs)
    void SetString(const std::string& key, const std::string& value);
    std::optional<std::string> GetString(const std::string& key) const;

    void SetInt(const std::string& key, int value);
    std::optional<int> GetInt(const std::string& key) const;

    void SetBool(const std::string& key, bool value);
    std::optional<bool> GetBool(const std::string& key) const;

    void SetFloat(const std::string& key, float value);
    std::optional<float> GetFloat(const std::string& key) const;

    // Check if key exists
    bool HasKey(const std::string& key) const;

    // Remove key
    void RemoveKey(const std::string& key);

    // Get all keys
    std::vector<std::string> GetAllKeys() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    // Helper to get config directory
    static std::filesystem::path GetConfigDirectory();

    // Helper to ensure config directory exists
    static bool EnsureConfigDirectoryExists();
};

} // namespace MetaImGUI

