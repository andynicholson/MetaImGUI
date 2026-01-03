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

#include "Application.h"
#include "Logger.h"

int main() {
    MetaImGUI::Application app;

    LOG_INFO("Initializing MetaImGUI...");

    if (!app.Initialize()) {
        LOG_ERROR("Failed to initialize application!");
        return -1;
    }

    LOG_INFO("Running MetaImGUI...");
    app.Run();

    LOG_INFO("Shutting down...");
    app.Shutdown();

    return 0;
}
