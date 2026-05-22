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

#include "Signal.h"

namespace MetaImGUI {

/**
 * @brief Decoupled UI-event bus shared between UIRenderer and Application.
 *
 * UIRenderer fires these signals from menu items and buttons; Application
 * connects slots that react (toggle dialogs, kick off async work, request
 * close). This keeps UIRenderer free of the std::function callback soup it
 * used to take as parameters and keeps event ownership in one obvious place.
 */
struct UIEvents {
    Signal<> exitRequested;
    Signal<> toggleDemoWindow;
    Signal<> showDemoWindow; // open-only — distinct from toggle so the "Show Demo" button doesn't close it
    Signal<> checkUpdatesRequested;
    Signal<> showAboutRequested;
    Signal<> showInputDialogRequested;
    Signal<> toggleISSTracker;
};

} // namespace MetaImGUI
