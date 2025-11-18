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

private:
    // GLFW callbacks (static because GLFW is C API)
    static void ErrorCallback(int error, const char* description);
    static void FramebufferSizeCallbackInternal(GLFWwindow* window, int width, int height);
    static void KeyCallbackInternal(GLFWwindow* window, int key, int scancode, int action, int mods);

    GLFWwindow* m_window;
    std::string m_title;
    int m_width;
    int m_height;
    bool m_initialized;

    // Callbacks
    std::function<void(int, int)> m_framebufferSizeCallback;
    std::function<void(int, int, int, int)> m_keyCallback;
};

} // namespace MetaImGUI
