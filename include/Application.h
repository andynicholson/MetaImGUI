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
namespace MetaImGUI {
class WindowManager;
class UIRenderer;
class UpdateChecker;
class ConfigManager;
class DialogManager;
class ISSTracker;
struct UpdateInfo;
} // namespace MetaImGUI

namespace MetaImGUI {

/**
 * @brief Main application class that orchestrates the application lifecycle
 *
 * The Application class follows the Single Responsibility Principle by
 * delegating window management to WindowManager and UI rendering to UIRenderer.
 * It focuses on high-level orchestration, state management, and business logic.
 *
 * @details
 * The Application class coordinates all major subsystems:
 * - WindowManager: GLFW window creation and event handling
 * - UIRenderer: ImGui rendering and UI logic
 * - ConfigManager: JSON-based configuration persistence
 * - Logger: Thread-safe logging system
 * - DialogManager: Reusable dialog components
 * - UpdateChecker: Asynchronous update notifications
 * - Localization: Multi-language support
 *
 * ## Usage Example
 * @code
 * int main() {
 *     MetaImGUI::Application app;
 *     if (!app.Initialize()) {
 *         return 1;
 *     }
 *     app.Run();  // Blocks until application closes
 *     app.Shutdown();
 *     return 0;
 * }
 * @endcode
 *
 * @see WindowManager, UIRenderer, ConfigManager
 */
class Application {
public:
    Application();
    ~Application();

    // Disable copy and move
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    /**
     * @brief Initialize the application and all subsystems
     *
     * Initializes all components in the correct order:
     * 1. Logger system
     * 2. Configuration manager (loads settings)
     * 3. Localization system (loads translations)
     * 4. Window manager (creates GLFW window)
     * 5. UI renderer (initializes ImGui)
     * 6. Dialog manager
     * 7. Update checker (starts async update check)
     *
     * @return true if all subsystems initialized successfully, false otherwise
     * @note Must be called before Run()
     * @see Shutdown()
     */
    bool Initialize();

    /**
     * @brief Run the main application loop
     *
     * Enters the main event loop which continues until the window is closed.
     * The loop processes:
     * - Window events (resize, input, etc.)
     * - ImGui frame rendering
     * - Application logic updates
     *
     * @note This method blocks until the application is closed
     * @note Must be called after successful Initialize()
     * @see ShouldClose()
     */
    void Run();

    /**
     * @brief Shutdown the application and cleanup resources
     *
     * Performs graceful shutdown of all subsystems in reverse order:
     * - Saves configuration (window size, preferences)
     * - Destroys update checker
     * - Destroys dialog manager
     * - Destroys UI renderer
     * - Destroys window manager
     * - Destroys configuration manager
     * - Shuts down logger
     *
     * @note Safe to call multiple times
     * @note Automatically called by destructor if not called explicitly
     */
    void Shutdown();

    /**
     * @brief Check if the application should close
     * @return true if the application should close
     */
    [[nodiscard]] bool ShouldClose() const;

private:
    // Subsystem managers
    std::unique_ptr<WindowManager> m_windowManager;
    std::unique_ptr<UIRenderer> m_uiRenderer;
    std::unique_ptr<UpdateChecker> m_updateChecker;
    std::unique_ptr<ConfigManager> m_configManager;
    std::unique_ptr<DialogManager> m_dialogManager;
    std::unique_ptr<ISSTracker> m_issTracker;

    // Application state
    bool m_initialized = false;
    bool m_showAboutWindow = false;
    bool m_showDemoWindow = false;
    bool m_showUpdateNotification = false;
    bool m_updateCheckInProgress = false;
    bool m_showExitDialog = false;
    bool m_showISSTracker = false;

    // Update checking
    std::unique_ptr<UpdateInfo> m_latestUpdateInfo;

    // Status bar state
    std::string m_statusMessage;
    float m_lastFrameTime = 0.0f;

    // Private methods
    void ProcessInput();
    void Render();
    void CheckForUpdates();
    void OnUpdateCheckComplete(const UpdateInfo& updateInfo);

    // Context recovery
    bool OnContextLoss();

    // Input callbacks
    void OnFramebufferSizeChanged(int width, int height);
    void OnKeyPressed(int key, int scancode, int action, int mods);
    void OnWindowCloseRequested();

    // UI event handlers
    void OnExitRequested();
    void OnToggleDemoWindow();
    void OnCheckUpdatesRequested();
    void OnShowAboutRequested();
    void OnShowInputDialogRequested();
    void OnToggleISSTracker();

    // Window size constants
    static constexpr int DEFAULT_WIDTH = 1200;
    static constexpr int DEFAULT_HEIGHT = 800;
    static constexpr const char* WINDOW_TITLE = "MetaImGUI - ImGui Application Template";
};

} // namespace MetaImGUI
