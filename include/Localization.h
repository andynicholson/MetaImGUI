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

    /**
     * @brief Set current language
     * @param languageCode ISO 639-1 language code (e.g., "en", "es", "fr")
     */
    void SetLanguage(const std::string& languageCode);

    /**
     * @brief Get current language code
     */
    std::string GetCurrentLanguage() const;

    /**
     * @brief Get list of available languages
     */
    std::vector<std::string> GetAvailableLanguages() const;

    /**
     * @brief Get translated string
     * @param key Translation key
     * @return Translated string (or key if not found)
     */
    std::string Tr(const std::string& key) const;

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

    /**
     * @brief Initialize with built-in translations
     */
    void InitializeBuiltInTranslations();

private:
    Localization();
    ~Localization() = default;

    std::string m_currentLanguage;
    std::map<std::string, std::map<std::string, std::string>> m_translations; // [language][key] = value
};

} // namespace MetaImGUI

// Convenience macro for translation
#define TR(key) MetaImGUI::Localization::Instance().Tr(key)
