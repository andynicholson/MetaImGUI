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
        m_trCache.clear(); // Stale: cached entries belong to the previous language.
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
    if (auto cacheIt = m_trCache.find(key); cacheIt != m_trCache.end()) {
        return cacheIt->second;
    }

    // Cache miss — walk the lookup tiers.
    auto resolve = [this, &key]() -> std::string {
        if (auto langIt = m_translations.find(m_currentLanguage); langIt != m_translations.end()) {
            if (auto keyIt = langIt->second.find(key); keyIt != langIt->second.end()) {
                return keyIt->second;
            }
        }
        if (m_currentLanguage != "en") {
            if (auto enIt = m_translations.find("en"); enIt != m_translations.end()) {
                if (auto keyIt = enIt->second.find(key); keyIt != enIt->second.end()) {
                    return keyIt->second;
                }
            }
        }
        return key; // Key not found — surface the key name to make the gap visible.
    };

    std::string resolved = resolve();
    // Memoize. We don't bound the cache size: the set of distinct UI keys is
    // small (dozens) and bounded by the JSON file, not by user input.
    m_trCache.emplace(key, resolved);
    return resolved;
}

void Localization::AddTranslation(const std::string& languageCode, const std::string& key, const std::string& value) {
    m_translations[languageCode][key] = value;
    // Invalidate just the affected cache entry rather than the whole cache,
    // so a streaming load doesn't blow away the cache repeatedly.
    if (languageCode == m_currentLanguage || languageCode == "en") {
        m_trCache.erase(key);
    }
}

bool Localization::LoadTranslations(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            // Application probes several candidate paths in order; a miss here
            // is the expected case until the right one is found, so log at
            // DEBUG only. The aggregated final-failure error is the caller's
            // responsibility.
            LOG_DEBUG("Translations file not found at: {}", filepath);
            return false;
        }

        json j = json::parse(file);

        // Wipe any previously-loaded translations on successful re-load so a
        // language whose key set shrank between loads doesn't leak stale
        // entries.
        m_translations.clear();
        m_trCache.clear();

        for (const auto& [languageCode, translations] : j.items()) {
            for (const auto& [key, value] : translations.items()) {
                AddTranslation(languageCode, key, value.get<std::string>());
            }
        }

        LOG_INFO("Loaded translations from: {}", filepath);
        return true;
    } catch (const std::exception& e) {
        // Parse error is genuinely a problem (file existed but was malformed),
        // so this stays ERROR.
        LOG_ERROR("Failed to load translations from {}: {}", filepath, e.what());
        return false;
    }
}

} // namespace MetaImGUI
