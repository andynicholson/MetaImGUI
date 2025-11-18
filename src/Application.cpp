#include "Application.h"

#include "UIRenderer.h"
#include "UpdateChecker.h"
#include "WindowManager.h"
#include "version.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <iostream>

namespace MetaImGUI {

Application::Application()
    : m_windowManager(nullptr), m_uiRenderer(nullptr), m_updateChecker(nullptr), m_initialized(false),
      m_showAboutWindow(false), m_showDemoWindow(false), m_showUpdateNotification(false),
      m_updateCheckInProgress(false), m_latestUpdateInfo(nullptr), m_statusMessage("Ready"), m_lastFrameTime(0.0f) {}

Application::~Application() {
    Shutdown();
}

bool Application::Initialize() {
    if (m_initialized) {
        return true;
    }

    std::cout << "Initializing application subsystems..." << std::endl;

    // Create and initialize window manager
    m_windowManager = std::make_unique<WindowManager>(WINDOW_TITLE, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    if (!m_windowManager->Initialize()) {
        std::cerr << "Failed to initialize window manager" << std::endl;
        return false;
    }

    // Set up window callbacks
    m_windowManager->SetFramebufferSizeCallback(
        [this](int width, int height) { this->OnFramebufferSizeChanged(width, height); });
    m_windowManager->SetKeyCallback(
        [this](int key, int scancode, int action, int mods) { this->OnKeyPressed(key, scancode, action, mods); });

    // Create and initialize UI renderer
    m_uiRenderer = std::make_unique<UIRenderer>();
    if (!m_uiRenderer->Initialize(m_windowManager->GetNativeWindow())) {
        std::cerr << "Failed to initialize UI renderer" << std::endl;
        return false;
    }

    // Initialize update checker
    // TODO: Replace with your GitHub username and repo name
    m_updateChecker = std::make_unique<UpdateChecker>("andynicholson", "MetaImGUI");

    // Check for updates asynchronously after a short delay
    // This is non-blocking and won't slow down startup
    CheckForUpdates();

    m_initialized = true;
    std::cout << "Application initialized successfully" << std::endl;
    return true;
}

void Application::Run() {
    while (!ShouldClose()) {
        ProcessInput();
        Render();
    }
}

void Application::Shutdown() {
    if (!m_initialized) {
        return;
    }

    std::cout << "Shutting down application..." << std::endl;

    // Shutdown subsystems in reverse order of initialization
    m_updateChecker.reset();
    m_uiRenderer.reset();
    m_windowManager.reset();

    m_initialized = false;
    std::cout << "Application shut down successfully" << std::endl;
}

bool Application::ShouldClose() const {
    return m_windowManager && m_windowManager->ShouldClose();
}

void Application::ProcessInput() {
    if (m_windowManager) {
        m_windowManager->PollEvents();
    }
}

void Application::Render() {
    if (!m_windowManager || !m_uiRenderer) {
        return;
    }

    // Get frame time for FPS calculation
    ImGuiIO& io = ImGui::GetIO();
    m_lastFrameTime = io.Framerate;

    // Prepare window for rendering
    m_windowManager->BeginFrame();

    // Start ImGui frame
    m_uiRenderer->BeginFrame();

    // Create full-screen main window
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
                                    ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    if (ImGui::Begin("MetaImGUI Main", nullptr, window_flags)) {
        // Render menu bar
        m_uiRenderer->RenderMenuBar([this]() { this->OnExitRequested(); }, [this]() { this->OnToggleDemoWindow(); },
                                    [this]() { this->OnCheckUpdatesRequested(); },
                                    [this]() { this->OnShowAboutRequested(); }, m_showDemoWindow);

        // Render main window content
        m_uiRenderer->RenderMainWindow([this]() { this->OnShowAboutRequested(); },
                                       [this]() { this->OnToggleDemoWindow(); });

        // Render status bar
        m_uiRenderer->RenderStatusBar(m_statusMessage, m_lastFrameTime, Version::VERSION, m_updateCheckInProgress);
    }
    ImGui::End();

    ImGui::PopStyleVar(3);

    // Render additional windows
    if (m_showAboutWindow) {
        m_uiRenderer->RenderAboutWindow(m_showAboutWindow);
    }

    if (m_showDemoWindow) {
        m_uiRenderer->ShowDemoWindow(m_showDemoWindow);
    }

    if (m_showUpdateNotification) {
        m_uiRenderer->RenderUpdateNotification(m_showUpdateNotification, m_latestUpdateInfo.get());
    }

    // End ImGui frame and render
    m_uiRenderer->EndFrame();

    // Present the frame
    m_windowManager->EndFrame();
}

// Event Handlers

void Application::OnExitRequested() {
    if (m_windowManager) {
        m_windowManager->RequestClose();
    }
}

void Application::OnToggleDemoWindow() {
    m_showDemoWindow = !m_showDemoWindow;
}

void Application::OnCheckUpdatesRequested() {
    CheckForUpdates();
}

void Application::OnShowAboutRequested() {
    m_showAboutWindow = true;
}

// Input Callbacks

void Application::OnFramebufferSizeChanged(int width, int height) {
    // Handle framebuffer size changes if needed
    // Currently handled automatically by WindowManager
    (void)width;
    (void)height;
}

void Application::OnKeyPressed(int key, int scancode, int action, int mods) {
    (void)scancode; // Unused parameter

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                OnExitRequested();
                break;
            case GLFW_KEY_A:
                if (mods & GLFW_MOD_CONTROL) {
                    OnShowAboutRequested();
                }
                break;
        }
    }
}

// Update Checking

void Application::CheckForUpdates() {
    if (!m_updateChecker || m_updateCheckInProgress) {
        return;
    }

    m_updateCheckInProgress = true;
    m_statusMessage = "Checking for updates...";

    // Check asynchronously
    m_updateChecker->CheckForUpdatesAsync([this](const UpdateInfo& info) { this->OnUpdateCheckComplete(info); });
}

void Application::OnUpdateCheckComplete(const UpdateInfo& updateInfo) {
    m_updateCheckInProgress = false;

    if (updateInfo.updateAvailable) {
        m_latestUpdateInfo = std::make_unique<UpdateInfo>(updateInfo);
        m_showUpdateNotification = true;
        m_statusMessage = "Update available: v" + updateInfo.latestVersion;

        std::cout << "Update available: v" << updateInfo.latestVersion << " (current: v" << updateInfo.currentVersion
                  << ")" << std::endl;
    } else {
        m_statusMessage = "Ready";
        std::cout << "No updates available (current version: v" << updateInfo.currentVersion << ")" << std::endl;
    }
}

} // namespace MetaImGUI
