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

namespace MetaImGUI {

/**
 * @brief Manages application themes and styling
 *
 * ThemeManager provides a centralized way to handle ImGui themes.
 * It includes built-in themes (Dark, Light, Classic) and a custom
 * Modern theme optimized for readability and professional appearance.
 */
class ThemeManager {
public:
    enum class Theme {
        Dark,    ///< ImGui's default dark theme
        Light,   ///< ImGui's default light theme
        Classic, ///< ImGui's classic theme
        Modern   ///< Custom light theme with rounded corners and borders
    };

    /**
     * @brief Apply a theme to the current ImGui context
     * @param theme The theme to apply
     */
    static void Apply(Theme theme);

    /**
     * @brief Get the currently active theme
     * @return The current theme
     */
    static Theme GetCurrent() {
        return s_currentTheme;
    }

private:
    static void ApplyModernTheme();
    static Theme s_currentTheme;
};

} // namespace MetaImGUI
