#pragma once

#include <functional>
#include <string>

// Forward declarations
struct GLFWwindow;

namespace MetaImGUI {

/**
 * @brief Manages GLFW window creation, lifecycle, and input handling
 *
 * WindowManager encapsulates all GLFW-specific operations, providing a clean
 * interface for window management without exposing GLFW details to the application.
 */
class WindowManager {
public:
    /**
     * @brief Construct a new Window Manager
     * @param title Window title
     * @param width Initial window width
     * @param height Initial window height
     */
    WindowManager(const std::string& title, int width, int height);
    ~WindowManager();

    // Disable copy and move
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    WindowManager(WindowManager&&) = delete;
    WindowManager& operator=(WindowManager&&) = delete;

    /**
     * @brief Initialize the window and GLFW
     * @return true if initialization succeeded, false otherwise
     */
    bool Initialize();

    /**
     * @brief Shutdown and cleanup the window
     */
    void Shutdown();

    /**
     * @brief Check if the window should close
     * @return true if the window should close
     */
    bool ShouldClose() const;

    /**
     * @brief Poll for input events
     */
    void PollEvents();

    /**
     * @brief Prepare the window for a new frame
     */
    void BeginFrame();

    /**
     * @brief Present the rendered frame
     */
    void EndFrame();

    /**
     * @brief Get the current framebuffer size
     * @param width Output: framebuffer width
     * @param height Output: framebuffer height
     */
    void GetFramebufferSize(int& width, int& height) const;

    /**
     * @brief Get the current window size
     * @param width Output: window width
     * @param height Output: window height
     */
    void GetWindowSize(int& width, int& height) const;

    /**
     * @brief Get the GLFW window pointer (for ImGui integration)
     * @return GLFWwindow pointer
     */
    GLFWwindow* GetNativeWindow() const {
        return m_window;
    }

    /**
     * @brief Request the window to close
     */
    void RequestClose();

    /**
     * @brief Set framebuffer size callback
     * @param callback Function to call when framebuffer size changes
     */
    void SetFramebufferSizeCallback(std::function<void(int, int)> callback);

    /**
     * @brief Set key input callback
     * @param callback Function to call when key is pressed
     */
    void SetKeyCallback(std::function<void(int, int, int, int)> callback);

    /**
     * @brief Set window close callback
     * @param callback Function to call when window close is requested
     */
    void SetWindowCloseCallback(std::function<void()> callback);

    /**
     * @brief Cancel a close request (clears the should close flag)
     */
    void CancelClose();

    /**
     * @brief Set context loss callback
     * @param callback Function to call when OpenGL context is lost (for recovery)
     * @return true if context was successfully recreated
     */
    void SetContextLossCallback(std::function<bool()> callback);

    /**
     * @brief Check if context is valid and attempt recovery if not
     * @return true if context is valid or was successfully recovered
     */
    bool ValidateContext();

private:
    /**
     * @brief Attempt to recreate the OpenGL context
     * @return true if context was successfully recreated
     */
    bool RecreateContext();
    // GLFW callbacks (static because GLFW is C API)
    static void ErrorCallback(int error, const char* description);
    static void FramebufferSizeCallbackInternal(GLFWwindow* window, int width, int height);
    static void KeyCallbackInternal(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void WindowCloseCallbackInternal(GLFWwindow* window);

    GLFWwindow* m_window;
    std::string m_title;
    int m_width;
    int m_height;
    bool m_initialized;

    // Context recovery
    int m_contextRecoveryAttempts;
    static constexpr int MAX_RECOVERY_ATTEMPTS = 3;

    // Callbacks
    std::function<void(int, int)> m_framebufferSizeCallback;
    std::function<void(int, int, int, int)> m_keyCallback;
    std::function<void()> m_windowCloseCallback;
    std::function<bool()> m_contextLossCallback;
};

} // namespace MetaImGUI
