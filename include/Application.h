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
struct UpdateInfo;
} // namespace MetaImGUI

namespace MetaImGUI {

/**
 * @brief Main application class that orchestrates the application lifecycle
 *
 * The Application class now follows the Single Responsibility Principle by
 * delegating window management to WindowManager and UI rendering to UIRenderer.
 * It focuses on high-level orchestration, state management, and business logic.
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
     * @return true if initialization succeeded
     */
    bool Initialize();

    /**
     * @brief Run the main application loop
     */
    void Run();

    /**
     * @brief Shutdown the application and cleanup resources
     */
    void Shutdown();

    /**
     * @brief Check if the application should close
     * @return true if the application should close
     */
    bool ShouldClose() const;

private:
    // Subsystem managers
    std::unique_ptr<WindowManager> m_windowManager;
    std::unique_ptr<UIRenderer> m_uiRenderer;
    std::unique_ptr<UpdateChecker> m_updateChecker;
    std::unique_ptr<ConfigManager> m_configManager;
    std::unique_ptr<DialogManager> m_dialogManager;

    // Application state
    bool m_initialized;
    bool m_showAboutWindow;
    bool m_showDemoWindow;
    bool m_showUpdateNotification;
    bool m_updateCheckInProgress;
    bool m_showExitDialog;

    // Update checking
    std::unique_ptr<UpdateInfo> m_latestUpdateInfo;

    // Status bar state
    std::string m_statusMessage;
    float m_lastFrameTime;

    // Private methods
    void ProcessInput();
    void Render();
    void CheckForUpdates();
    void OnUpdateCheckComplete(const UpdateInfo& updateInfo);

    // Input callbacks
    void OnFramebufferSizeChanged(int width, int height);
    void OnKeyPressed(int key, int scancode, int action, int mods);
    void OnWindowCloseRequested();

    // UI event handlers
    void OnExitRequested();
    void OnToggleDemoWindow();
    void OnCheckUpdatesRequested();
    void OnShowAboutRequested();

    // Window size constants
    static constexpr int DEFAULT_WIDTH = 1200;
    static constexpr int DEFAULT_HEIGHT = 800;
    static constexpr const char* WINDOW_TITLE = "MetaImGUI - ImGui Application Template";
};

} // namespace MetaImGUI
