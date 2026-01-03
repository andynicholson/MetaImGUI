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
    [[nodiscard]] bool ConfigFileExists() const;

    /**
     * @brief Get configuration file path
     */
    [[nodiscard]] std::filesystem::path GetConfigPath() const;

    // Window settings
    void SetWindowPosition(int x, int y);
    void SetWindowSize(int width, int height);
    [[nodiscard]] std::optional<std::pair<int, int>> GetWindowPosition() const;
    [[nodiscard]] std::optional<std::pair<int, int>> GetWindowSize() const;
    void SetWindowMaximized(bool maximized);
    [[nodiscard]] bool GetWindowMaximized() const;

    // Theme settings
    void SetTheme(const std::string& theme);
    [[nodiscard]] std::string GetTheme() const;

    // Recent files
    void AddRecentFile(const std::string& filepath);
    [[nodiscard]] std::vector<std::string> GetRecentFiles() const;
    void ClearRecentFiles();
    void SetMaxRecentFiles(size_t max);

    // Generic settings (string key-value pairs)
    void SetString(const std::string& key, const std::string& value);
    [[nodiscard]] std::optional<std::string> GetString(const std::string& key) const;

    void SetInt(const std::string& key, int value);
    [[nodiscard]] std::optional<int> GetInt(const std::string& key) const;

    void SetBool(const std::string& key, bool value);
    [[nodiscard]] std::optional<bool> GetBool(const std::string& key) const;

    void SetFloat(const std::string& key, float value);
    [[nodiscard]] std::optional<float> GetFloat(const std::string& key) const;

    // Check if key exists
    [[nodiscard]] bool HasKey(const std::string& key) const;

    // Remove key
    void RemoveKey(const std::string& key);

    // Get all keys
    [[nodiscard]] std::vector<std::string> GetAllKeys() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    // Helper to get config directory
    static std::filesystem::path GetConfigDirectory();

    // Helper to ensure config directory exists
    static bool EnsureConfigDirectoryExists();
};

} // namespace MetaImGUI
