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

#include <map>
#include <string>
#include <vector>

namespace MetaImGUI {

/**
 * @brief Simple localization/internationalization system
 *
 * Provides translation services for UI strings using key-value pairs.
 * Supports multiple languages with fallback to English.
 *
 * IMPORTANT: All translations MUST come from resources/translations/translations.json
 * DO NOT add built-in translations here. This is the single source of truth.
 */
class Localization {
public:
    /**
     * @brief Get singleton instance
     */
    static Localization& Instance();

    // Delete copy and move
    Localization(const Localization&) = delete;
    Localization& operator=(const Localization&) = delete;
    Localization(Localization&&) = delete;
    Localization& operator=(Localization&&) = delete;

    /**
     * @brief Set current language
     * @param languageCode ISO 639-1 language code (e.g., "en", "es", "fr")
     */
    void SetLanguage(const std::string& languageCode);

    /**
     * @brief Get current language code
     */
    [[nodiscard]] std::string GetCurrentLanguage() const;

    /**
     * @brief Get list of available languages
     */
    [[nodiscard]] std::vector<std::string> GetAvailableLanguages() const;

    /**
     * @brief Get translated string
     * @param key Translation key
     * @return Translated string (or key if not found)
     */
    [[nodiscard]] std::string Tr(const std::string& key) const;

    /**
     * @brief Add translation for a language
     * @param languageCode Language code
     * @param key Translation key
     * @param value Translated string
     */
    void AddTranslation(const std::string& languageCode, const std::string& key, const std::string& value);

    /**
     * @brief Load translations from JSON file
     * @param filepath Path to JSON translation file
     * @return true if successful
     */
    bool LoadTranslations(const std::string& filepath);

private:
    Localization();
    ~Localization() = default;

    std::string m_currentLanguage;
    std::map<std::string, std::map<std::string, std::string>> m_translations; // [language][key] = value
};

} // namespace MetaImGUI

// Convenience macro for translation
#define TR(key) MetaImGUI::Localization::Instance().Tr(key)
