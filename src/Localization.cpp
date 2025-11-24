#include "Localization.h"

#include "Logger.h"

#include <nlohmann/json.hpp>

#include <fstream>

using json = nlohmann::json;

namespace MetaImGUI {

// ============================================================================
// WARNING: DO NOT ADD BUILT-IN TRANSLATIONS HERE!
// ============================================================================
// All translations MUST come from resources/translations/translations.json
// This is the SINGLE SOURCE OF TRUTH for all translations.
//
// Having two sources of translations (built-in + JSON) causes:
// - Maintenance nightmares (keeping them in sync)
// - Bugs when one is updated but not the other
// - Confusion about which translation is actually being used
//
// If translations.json fails to load, the application should:
// 1. Log a clear ERROR message
// 2. Return translation keys as-is (they serve as English fallback)
// 3. Be obvious to developers/users that something is wrong
// ============================================================================

Localization::Localization() : m_currentLanguage("en") {
    // Translations are loaded via LoadTranslations() - see warning above
}

Localization& Localization::Instance() {
    static Localization instance;
    return instance;
}

void Localization::SetLanguage(const std::string& languageCode) {
    if (m_translations.find(languageCode) != m_translations.end()) {
        m_currentLanguage = languageCode;
        LOG_INFO("Language set to: {}", languageCode);
    } else {
        LOG_ERROR("Language not available: {}", languageCode);
    }
}

std::string Localization::GetCurrentLanguage() const {
    return m_currentLanguage;
}

std::vector<std::string> Localization::GetAvailableLanguages() const {
    std::vector<std::string> languages;
    for (const auto& pair : m_translations) {
        languages.push_back(pair.first);
    }
    return languages;
}

std::string Localization::Tr(const std::string& key) const {
    // Try current language
    auto langIt = m_translations.find(m_currentLanguage);
    if (langIt != m_translations.end()) {
        auto keyIt = langIt->second.find(key);
        if (keyIt != langIt->second.end()) {
            return keyIt->second;
        }
    }

    // Fallback to English
    if (m_currentLanguage != "en") {
        auto enIt = m_translations.find("en");
        if (enIt != m_translations.end()) {
            auto keyIt = enIt->second.find(key);
            if (keyIt != enIt->second.end()) {
                return keyIt->second;
            }
        }
    }

    // Return key if not found
    return key;
}

void Localization::AddTranslation(const std::string& languageCode, const std::string& key, const std::string& value) {
    m_translations[languageCode][key] = value;
}

bool Localization::LoadTranslations(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open translations file: {}", filepath);
            return false;
        }

        json j = json::parse(file);

        for (auto& [languageCode, translations] : j.items()) {
            for (auto& [key, value] : translations.items()) {
                AddTranslation(languageCode, key, value.get<std::string>());
            }
        }

        LOG_INFO("Loaded translations from: {}", filepath);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load translations: {}", e.what());
        return false;
    }
}

} // namespace MetaImGUI
