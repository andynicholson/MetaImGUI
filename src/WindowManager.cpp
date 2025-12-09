#include "WindowManager.h"

#include "Logger.h"

#include <GLFW/glfw3.h>

namespace MetaImGUI {

WindowManager::WindowManager(std::string title, int width, int height)
    : m_title(std::move(title)), m_width(width), m_height(height) {}

WindowManager::~WindowManager() {
    Shutdown();
}

bool WindowManager::Initialize() {
    if (m_initialized) {
        return true;
    }

    // Initialize GLFW
    glfwSetErrorCallback(ErrorCallback);
    if (glfwInit() == GLFW_FALSE) {
        LOG_ERROR("Failed to initialize GLFW");
        return false;
    }

    // Set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (m_window == nullptr) {
        LOG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    // Store WindowManager pointer in GLFW window for callbacks
    glfwSetWindowUserPointer(m_window, this);

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Enable vsync

    // Print OpenGL information
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);

    if (version != nullptr) {
        LOG_INFO("OpenGL version: {}", reinterpret_cast<const char*>(version));
    } else {
        LOG_ERROR("Failed to get OpenGL version");
    }

    if (vendor != nullptr) {
        LOG_INFO("OpenGL vendor: {}", reinterpret_cast<const char*>(vendor));
    } else {
        LOG_ERROR("Failed to get OpenGL vendor");
    }

    if (renderer != nullptr) {
        LOG_INFO("OpenGL renderer: {}", reinterpret_cast<const char*>(renderer));
    } else {
        LOG_ERROR("Failed to get OpenGL renderer");
    }

    LOG_INFO("OpenGL context ready");

    m_initialized = true;
    return true;
}

void WindowManager::Shutdown() {
    if (!m_initialized) {
        return;
    }

    if (m_window != nullptr) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    glfwTerminate();
    m_initialized = false;
}

bool WindowManager::ShouldClose() const {
    return m_window == nullptr || (glfwWindowShouldClose(m_window) == GLFW_TRUE);
}

void WindowManager::PollEvents() {
    glfwPollEvents();
}

void WindowManager::BeginFrame() {
    if (m_window == nullptr) {
        LOG_ERROR("BeginFrame called with null window");
        return;
    }

    // Validate context and attempt recovery if needed
    if (!ValidateContext()) {
        LOG_ERROR("BeginFrame: Context validation failed - skipping frame");
        return;
    }

    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    m_width = width;
    m_height = height;

    glViewport(0, 0, m_width, m_height);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Check for errors after rendering operations
    const GLenum error = glGetError();
    if (error == GL_CONTEXT_LOST || error == GL_OUT_OF_MEMORY) {
        LOG_WARNING("OpenGL error during clear (0x{:X}) - will attempt recovery next frame", error);
        // Don't attempt immediate recovery here - let ValidateContext handle it next frame
    }
}

void WindowManager::EndFrame() {
    if (m_window == nullptr) {
        LOG_ERROR("EndFrame called with null window");
        return;
    }

    // Validate context before swapping (final check for this frame)
    if (!ValidateContext()) {
        LOG_ERROR("EndFrame: Context validation failed - skipping swap");
        return;
    }

    glfwSwapBuffers(m_window);
}

void WindowManager::GetFramebufferSize(int& width, int& height) const {
    if (m_window != nullptr) {
        glfwGetFramebufferSize(m_window, &width, &height);
    } else {
        width = m_width;
        height = m_height;
    }
}

void WindowManager::GetWindowSize(int& width, int& height) const {
    if (m_window != nullptr) {
        glfwGetWindowSize(m_window, &width, &height);
    } else {
        width = m_width;
        height = m_height;
    }
}

void WindowManager::RequestClose() {
    if (m_window != nullptr) {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }
}

void WindowManager::SetFramebufferSizeCallback(std::function<void(int, int)> callback) {
    m_framebufferSizeCallback = callback;
    if (m_window != nullptr) {
        glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallbackInternal);
    }
}

void WindowManager::SetKeyCallback(std::function<void(int, int, int, int)> callback) {
    m_keyCallback = callback;
    if (m_window != nullptr) {
        glfwSetKeyCallback(m_window, KeyCallbackInternal);
    }
}

void WindowManager::SetWindowCloseCallback(std::function<void()> callback) {
    m_windowCloseCallback = callback;
    if (m_window != nullptr) {
        glfwSetWindowCloseCallback(m_window, WindowCloseCallbackInternal);
    }
}

void WindowManager::CancelClose() {
    if (m_window != nullptr) {
        glfwSetWindowShouldClose(m_window, GLFW_FALSE);
    }
}

void WindowManager::SetContextLossCallback(std::function<bool()> callback) {
    m_contextLossCallback = callback;
}

bool WindowManager::ValidateContext() {
    if (m_window == nullptr) {
        return false;
    }

    // DEBUG: Simulate context loss for testing
    // Uncomment these lines to force context loss after 100 frames
    // static int frameCount = 0;
    // if (++frameCount == 100) {
    //     LOG_WARNING("DEBUG: Simulating context loss for testing");
    //     return RecreateContext();
    // }

    // Check if context is still valid
    if (glfwGetCurrentContext() != m_window) {
        LOG_WARNING("OpenGL context is no longer valid - attempting recovery");
        return RecreateContext();
    }

    // Check for OpenGL errors that indicate context loss
    const GLenum error = glGetError();
    if (error == GL_CONTEXT_LOST || error == GL_OUT_OF_MEMORY) {
        LOG_WARNING("OpenGL context lost (error: 0x{:X}) - attempting recovery", error);
        return RecreateContext();
    }

    // Context is valid
    m_contextRecoveryAttempts = 0; // Reset on success
    return true;
}

bool WindowManager::RecreateContext() {
    m_contextRecoveryAttempts++;

    if (m_contextRecoveryAttempts > MAX_RECOVERY_ATTEMPTS) {
        LOG_ERROR("Failed to recover OpenGL context after {} attempts - requesting window close",
                  MAX_RECOVERY_ATTEMPTS);
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
        return false;
    }

    LOG_INFO("Attempting to recreate OpenGL context (attempt {}/{})", m_contextRecoveryAttempts, MAX_RECOVERY_ATTEMPTS);

    // Make context current again (might have been lost)
    glfwMakeContextCurrent(m_window);

    // Check if making it current worked
    if (glfwGetCurrentContext() != m_window) {
        LOG_ERROR("Failed to make context current");
        return false;
    }

    // Clear any OpenGL errors
    while (glGetError() != GL_NO_ERROR) {
        // Drain error queue
    }

    // Call application-level recovery callback if set
    // This allows Application to recreate ImGui/ImPlot contexts
    if (m_contextLossCallback) {
        LOG_INFO("Calling context loss callback for application-level recovery");
        if (!m_contextLossCallback()) {
            LOG_ERROR("Application-level context recovery failed");
            return false;
        }
    }

    LOG_INFO("OpenGL context successfully recovered");
    m_contextRecoveryAttempts = 0; // Reset on successful recovery
    return true;
}

// Static callbacks

void WindowManager::ErrorCallback(int error, const char* description) {
    LOG_ERROR("GLFW Error {}: {}", error, description);
}

void WindowManager::FramebufferSizeCallbackInternal(GLFWwindow* window, int width, int height) {
    auto* manager = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (manager != nullptr && manager->m_framebufferSizeCallback) {
        manager->m_framebufferSizeCallback(width, height);
    }
}

void WindowManager::KeyCallbackInternal(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* manager = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (manager != nullptr && manager->m_keyCallback) {
        manager->m_keyCallback(key, scancode, action, mods);
    }
}

void WindowManager::WindowCloseCallbackInternal(GLFWwindow* window) {
    auto* manager = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (manager != nullptr && manager->m_windowCloseCallback) {
        manager->m_windowCloseCallback();
    }
}

} // namespace MetaImGUI
