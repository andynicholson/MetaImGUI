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
    if (m_translations.contains(languageCode)) {
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
    languages.reserve(m_translations.size());
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

        for (const auto& [languageCode, translations] : j.items()) {
            for (const auto& [key, value] : translations.items()) {
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
