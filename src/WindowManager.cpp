#include "WindowManager.h"

#include <GLFW/glfw3.h>

#include <iostream>

namespace MetaImGUI {

WindowManager::WindowManager(const std::string& title, int width, int height)
    : m_window(nullptr), m_title(title), m_width(width), m_height(height), m_initialized(false) {}

WindowManager::~WindowManager() {
    Shutdown();
}

bool WindowManager::Initialize() {
    if (m_initialized) {
        return true;
    }

    // Initialize GLFW
    glfwSetErrorCallback(ErrorCallback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
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
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    // Store WindowManager pointer in GLFW window for callbacks
    glfwSetWindowUserPointer(m_window, this);

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Enable vsync

    // Print OpenGL information
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL context ready" << std::endl;

    m_initialized = true;
    return true;
}

void WindowManager::Shutdown() {
    if (!m_initialized) {
        return;
    }

    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    glfwTerminate();
    m_initialized = false;
}

bool WindowManager::ShouldClose() const {
    return !m_window || glfwWindowShouldClose(m_window);
}

void WindowManager::PollEvents() {
    glfwPollEvents();
}

void WindowManager::BeginFrame() {
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    m_width = width;
    m_height = height;

    glViewport(0, 0, m_width, m_height);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void WindowManager::EndFrame() {
    glfwSwapBuffers(m_window);
}

void WindowManager::GetFramebufferSize(int& width, int& height) const {
    if (m_window) {
        glfwGetFramebufferSize(m_window, &width, &height);
    } else {
        width = m_width;
        height = m_height;
    }
}

void WindowManager::GetWindowSize(int& width, int& height) const {
    if (m_window) {
        glfwGetWindowSize(m_window, &width, &height);
    } else {
        width = m_width;
        height = m_height;
    }
}

void WindowManager::RequestClose() {
    if (m_window) {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }
}

void WindowManager::SetFramebufferSizeCallback(std::function<void(int, int)> callback) {
    m_framebufferSizeCallback = callback;
    if (m_window) {
        glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallbackInternal);
    }
}

void WindowManager::SetKeyCallback(std::function<void(int, int, int, int)> callback) {
    m_keyCallback = callback;
    if (m_window) {
        glfwSetKeyCallback(m_window, KeyCallbackInternal);
    }
}

// Static callbacks

void WindowManager::ErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void WindowManager::FramebufferSizeCallbackInternal(GLFWwindow* window, int width, int height) {
    WindowManager* manager = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (manager && manager->m_framebufferSizeCallback) {
        manager->m_framebufferSizeCallback(width, height);
    }
}

void WindowManager::KeyCallbackInternal(GLFWwindow* window, int key, int scancode, int action, int mods) {
    WindowManager* manager = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (manager && manager->m_keyCallback) {
        manager->m_keyCallback(key, scancode, action, mods);
    }
}

} // namespace MetaImGUI
