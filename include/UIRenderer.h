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

#include <memory>
#include <string>

// Forward declarations
struct GLFWwindow;

namespace MetaImGUI {

// Forward declarations
struct UpdateInfo;
struct UIEvents;
class ISSTracker;

/**
 * @brief Handles all ImGui rendering operations
 *
 * UIRenderer encapsulates all ImGui-specific rendering logic, keeping the
 * Application class focused on high-level orchestration and state management.
 */
class UIRenderer {
public:
    UIRenderer();
    ~UIRenderer();

    // Disable copy and move
    UIRenderer(const UIRenderer&) = delete;
    UIRenderer& operator=(const UIRenderer&) = delete;
    UIRenderer(UIRenderer&&) = delete;
    UIRenderer& operator=(UIRenderer&&) = delete;

    /**
     * @brief Initialize ImGui context and backends
     * @param window GLFW window pointer
     * @return true if initialization succeeded
     */
    bool Initialize(GLFWwindow* window);

    /**
     * @brief Shutdown ImGui context
     */
    void Shutdown();

    /**
     * @brief Begin a new ImGui frame
     */
    void BeginFrame();

    /**
     * @brief End the current ImGui frame and render
     */
    void EndFrame();

    /**
     * @brief Render the main application window
     * @param events UI event bus — Show About / Show Demo / Show Input fire signals here
     */
    void RenderMainWindow(UIEvents& events);

    /**
     * @brief Render the menu bar
     * @param events UI event bus — menu items fire signals on this object
     * @param showDemoWindow Current state of demo window visibility (for tick mark)
     * @param showISSTracker Current state of ISS tracker window visibility (for tick mark)
     */
    void RenderMenuBar(UIEvents& events, bool showDemoWindow, bool showISSTracker);

    /**
     * @brief Render the status bar
     * @param statusMessage Status message to display
     * @param fps Current FPS
     * @param version Version string
     * @param updateInProgress Whether an update check is in progress
     */
    void RenderStatusBar(const std::string& statusMessage, float fps, const char* version, bool updateInProgress);

    /**
     * @brief Render the about dialog
     * @param showAboutWindow Reference to visibility flag
     */
    void RenderAboutWindow(bool& showAboutWindow);

    /**
     * @brief Render the update notification dialog
     * @param showUpdateNotification Reference to visibility flag
     * @param updateInfo Pointer to update information
     */
    void RenderUpdateNotification(bool& showUpdateNotification, UpdateInfo* updateInfo);

    /**
     * @brief Show ImGui demo window
     * @param showDemoWindow Reference to visibility flag
     */
    void ShowDemoWindow(bool& showDemoWindow);

    /**
     * @brief Render the ISS tracker window
     * @param showISSTracker Reference to visibility flag
     * @param issTracker Pointer to ISSTracker instance
     */
    void RenderISSTrackerWindow(bool& showISSTracker, ISSTracker* issTracker);

    /**
     * @brief Helper to show tooltip with question mark
     * @param desc Tooltip description text
     */
    static void HelpMarker(const char* desc);

private:
    bool m_initialized = false;
};

} // namespace MetaImGUI
