#pragma once

#include <functional>
#include <memory>
#include <string>

// Forward declarations
struct GLFWwindow;

namespace MetaImGUI {

// Forward declarations
struct UpdateInfo;

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
     * @param onShowAbout Callback when "Show About" is clicked
     * @param onShowDemo Callback when "Show Demo" is clicked
     * @param onShowInputDialog Callback when "Show Input Dialog" is clicked
     */
    void RenderMainWindow(std::function<void()> onShowAbout, std::function<void()> onShowDemo,
                          std::function<void()> onShowInputDialog = nullptr);

    /**
     * @brief Render the menu bar
     * @param onExit Callback when exit is requested
     * @param onToggleDemo Callback when demo window is toggled
     * @param onCheckUpdates Callback when update check is requested
     * @param onShowAbout Callback when about is requested
     * @param showDemoWindow Current state of demo window visibility
     */
    void RenderMenuBar(std::function<void()> onExit, std::function<void()> onToggleDemo,
                       std::function<void()> onCheckUpdates, std::function<void()> onShowAbout, bool showDemoWindow);

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
     * @brief Helper to show tooltip with question mark
     * @param desc Tooltip description text
     */
    static void HelpMarker(const char* desc);

private:
    bool m_initialized;
};

} // namespace MetaImGUI
