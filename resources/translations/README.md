# Translation Files

## Single Source of Truth

**`translations.json`** in this directory is the **SINGLE SOURCE OF TRUTH** for all translations.

### ⚠️ IMPORTANT: Do NOT create duplicate translation files elsewhere!

All translations should be edited in this file only:
```
resources/translations/translations.json
```

### How it Works

The build system automatically copies this file to the appropriate locations:

1. **During Build** (CMakeLists.txt lines 209-218):
   - Development builds: Copied to `build/resources/translations/`
   - macOS bundles: Copied to `build/MetaImGUI.app/Contents/Resources/resources/translations/`

2. **During Install** (CMakeLists.txt lines 249-252):
   - Linux: Installed to `share/MetaImGUI/resources/translations/`
   - System paths: `/usr/share/MetaImGUI/resources/translations/`

3. **During Packaging**:
   - Windows installer: Copies from `resources/translations/` (windows_installer.nsi line 85)
   - Windows portable: Copies from `resources/translations/` (create_windows_installer.ps1 line 131)
   - Linux AppImage: Copies from `resources/` (release.yml line 151)
   - macOS DMG: Uses files already in app bundle (create_macos_dmg.sh line 63)

### Runtime Loading

The application tries multiple paths in this order (see Application.cpp):
1. `resources/translations/translations.json` (development/local build)
2. AppImage-specific path (if METAIMGUI_APPDIR is set)
3. macOS bundle path (relative to Contents/MacOS/)
4. System install paths (`../share/`, `/usr/share/`, `/usr/local/share/`)

### Adding New Translations

To add a new translation string:

1. Edit **only** `resources/translations/translations.json`
2. Add the key to all languages (en, es, fr, de)
3. Rebuild the project - changes will be automatically copied

Example:
```json
{
  "en": {
    "my.new.key": "My New String",
    ...
  },
  "es": {
    "my.new.key": "Mi Nueva Cadena",
    ...
  },
  ...
}
```

### Using Translations in Code

```cpp
auto& loc = Localization::Instance();
std::string translated = loc.Tr("my.new.key");

// In ImGui:
ImGui::Button(loc.Tr("button.ok").c_str());
```

### Supported Languages

Currently supported languages:
- `en` - English
- `es` - Spanish (Español)
- `fr` - French (Français)
- `de` - German (Deutsch)

To add a new language, add a new language section to the JSON file and update the language menu in UIRenderer.cpp.


