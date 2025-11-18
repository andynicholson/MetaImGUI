#include "Localization.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace MetaImGUI {

Localization::Localization() : m_currentLanguage("en") {
    InitializeBuiltInTranslations();
}

Localization& Localization::Instance() {
    static Localization instance;
    return instance;
}

void Localization::SetLanguage(const std::string& languageCode) {
    if (m_translations.find(languageCode) != m_translations.end()) {
        m_currentLanguage = languageCode;
        std::cout << "Language set to: " << languageCode << std::endl;
    } else {
        std::cerr << "Language not available: " << languageCode << std::endl;
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
            std::cerr << "Failed to open translations file: " << filepath << std::endl;
            return false;
        }

        json j = json::parse(file);

        for (auto& [languageCode, translations] : j.items()) {
            for (auto& [key, value] : translations.items()) {
                AddTranslation(languageCode, key, value.get<std::string>());
            }
        }

        std::cout << "Loaded translations from: " << filepath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load translations: " << e.what() << std::endl;
        return false;
    }
}

void Localization::InitializeBuiltInTranslations() {
    // English (default)
    AddTranslation("en", "app.title", "MetaImGUI - ImGui Application Template");
    AddTranslation("en", "app.welcome", "Welcome to MetaImGUI!");
    AddTranslation("en", "app.description", "A modern C++20 template for creating ImGui-based desktop applications.");

    // Menu
    AddTranslation("en", "menu.file", "File");
    AddTranslation("en", "menu.edit", "Edit");
    AddTranslation("en", "menu.view", "View");
    AddTranslation("en", "menu.help", "Help");
    AddTranslation("en", "menu.exit", "Exit");
    AddTranslation("en", "menu.about", "About");
    AddTranslation("en", "menu.settings", "Settings");
    AddTranslation("en", "menu.check_updates", "Check for Updates");
    AddTranslation("en", "menu.demo_window", "Show Demo Window");

    // Dialogs
    AddTranslation("en", "dialog.ok", "OK");
    AddTranslation("en", "dialog.cancel", "Cancel");
    AddTranslation("en", "dialog.yes", "Yes");
    AddTranslation("en", "dialog.no", "No");
    AddTranslation("en", "dialog.close", "Close");
    AddTranslation("en", "dialog.save", "Save");
    AddTranslation("en", "dialog.open", "Open");

    // Status
    AddTranslation("en", "status.ready", "Ready");
    AddTranslation("en", "status.loading", "Loading...");
    AddTranslation("en", "status.saving", "Saving...");
    AddTranslation("en", "status.error", "Error");

    // Spanish
    AddTranslation("es", "app.title", "MetaImGUI - Plantilla de Aplicación ImGui");
    AddTranslation("es", "app.welcome", "¡Bienvenido a MetaImGUI!");
    AddTranslation("es", "app.description",
                   "Una plantilla moderna de C++20 para crear aplicaciones de escritorio basadas en ImGui.");

    AddTranslation("es", "menu.file", "Archivo");
    AddTranslation("es", "menu.edit", "Editar");
    AddTranslation("es", "menu.view", "Ver");
    AddTranslation("es", "menu.help", "Ayuda");
    AddTranslation("es", "menu.exit", "Salir");
    AddTranslation("es", "menu.about", "Acerca de");
    AddTranslation("es", "menu.settings", "Configuración");
    AddTranslation("es", "menu.check_updates", "Buscar Actualizaciones");
    AddTranslation("es", "menu.demo_window", "Mostrar Ventana Demo");

    AddTranslation("es", "dialog.ok", "Aceptar");
    AddTranslation("es", "dialog.cancel", "Cancelar");
    AddTranslation("es", "dialog.yes", "Sí");
    AddTranslation("es", "dialog.no", "No");
    AddTranslation("es", "dialog.close", "Cerrar");
    AddTranslation("es", "dialog.save", "Guardar");
    AddTranslation("es", "dialog.open", "Abrir");

    AddTranslation("es", "status.ready", "Listo");
    AddTranslation("es", "status.loading", "Cargando...");
    AddTranslation("es", "status.saving", "Guardando...");
    AddTranslation("es", "status.error", "Error");

    // French
    AddTranslation("fr", "app.title", "MetaImGUI - Modèle d'Application ImGui");
    AddTranslation("fr", "app.welcome", "Bienvenue dans MetaImGUI!");
    AddTranslation("fr", "app.description",
                   "Un modèle C++20 moderne pour créer des applications de bureau basées sur ImGui.");

    AddTranslation("fr", "menu.file", "Fichier");
    AddTranslation("fr", "menu.edit", "Éditer");
    AddTranslation("fr", "menu.view", "Affichage");
    AddTranslation("fr", "menu.help", "Aide");
    AddTranslation("fr", "menu.exit", "Quitter");
    AddTranslation("fr", "menu.about", "À propos");
    AddTranslation("fr", "menu.settings", "Paramètres");
    AddTranslation("fr", "menu.check_updates", "Vérifier les Mises à Jour");
    AddTranslation("fr", "menu.demo_window", "Afficher la Fenêtre Démo");

    AddTranslation("fr", "dialog.ok", "OK");
    AddTranslation("fr", "dialog.cancel", "Annuler");
    AddTranslation("fr", "dialog.yes", "Oui");
    AddTranslation("fr", "dialog.no", "Non");
    AddTranslation("fr", "dialog.close", "Fermer");
    AddTranslation("fr", "dialog.save", "Enregistrer");
    AddTranslation("fr", "dialog.open", "Ouvrir");

    AddTranslation("fr", "status.ready", "Prêt");
    AddTranslation("fr", "status.loading", "Chargement...");
    AddTranslation("fr", "status.saving", "Sauvegarde...");
    AddTranslation("fr", "status.error", "Erreur");

    // German
    AddTranslation("de", "app.title", "MetaImGUI - ImGui Anwendungsvorlage");
    AddTranslation("de", "app.welcome", "Willkommen bei MetaImGUI!");
    AddTranslation("de", "app.description",
                   "Eine moderne C++20-Vorlage zum Erstellen von ImGui-basierten Desktop-Anwendungen.");

    AddTranslation("de", "menu.file", "Datei");
    AddTranslation("de", "menu.edit", "Bearbeiten");
    AddTranslation("de", "menu.view", "Ansicht");
    AddTranslation("de", "menu.help", "Hilfe");
    AddTranslation("de", "menu.exit", "Beenden");
    AddTranslation("de", "menu.about", "Über");
    AddTranslation("de", "menu.settings", "Einstellungen");
    AddTranslation("de", "menu.check_updates", "Nach Updates suchen");
    AddTranslation("de", "menu.demo_window", "Demo-Fenster anzeigen");

    AddTranslation("de", "dialog.ok", "OK");
    AddTranslation("de", "dialog.cancel", "Abbrechen");
    AddTranslation("de", "dialog.yes", "Ja");
    AddTranslation("de", "dialog.no", "Nein");
    AddTranslation("de", "dialog.close", "Schließen");
    AddTranslation("de", "dialog.save", "Speichern");
    AddTranslation("de", "dialog.open", "Öffnen");

    AddTranslation("de", "status.ready", "Bereit");
    AddTranslation("de", "status.loading", "Laden...");
    AddTranslation("de", "status.saving", "Speichern...");
    AddTranslation("de", "status.error", "Fehler");
}

} // namespace MetaImGUI
