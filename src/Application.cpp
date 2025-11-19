#include "Application.h"

#include "ConfigManager.h"
#include "DialogManager.h"
#include "Localization.h"
#include "Logger.h"
#include "UIRenderer.h"
#include "UpdateChecker.h"
#include "WindowManager.h"
#include "version.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <iostream>

namespace MetaImGUI {

Application::Application()
    : m_windowManager(nullptr), m_uiRenderer(nullptr), m_updateChecker(nullptr), m_configManager(nullptr),
      m_dialogManager(nullptr), m_initialized(false), m_showAboutWindow(false), m_showDemoWindow(false),
      m_showUpdateNotification(false), m_updateCheckInProgress(false), m_showExitDialog(false),
      m_latestUpdateInfo(nullptr), m_statusMessage("Ready"), m_lastFrameTime(0.0f) {}

Application::~Application() {
    Shutdown();
}

bool Application::Initialize() {
    if (m_initialized) {
        return true;
    }

    // Initialize logger first
    Logger::Instance().Initialize("logs/metaimgui.log", LogLevel::Info);
    LOG_INFO("Initializing MetaImGUI v{}", Version::VERSION);

    // Load configuration
    m_configManager = std::make_unique<ConfigManager>();
    if (m_configManager->Load()) {
        LOG_INFO("Configuration loaded successfully");
    } else {
        LOG_INFO("Using default configuration");
    }

    // Load translations and set language from config
    if (!Localization::Instance().LoadTranslations("resources/translations/translations.json")) {
        LOG_ERROR("Failed to load translations, using fallback keys");
    }
    std::string language = m_configManager->GetString("language").value_or("en");
    Localization::Instance().SetLanguage(language);
    LOG_INFO("Language set to: {}", language);

    // Create and initialize window manager
    auto windowSize = m_configManager->GetWindowSize();
    int width = windowSize ? windowSize->first : DEFAULT_WIDTH;
    int height = windowSize ? windowSize->second : DEFAULT_HEIGHT;

    m_windowManager = std::make_unique<WindowManager>(WINDOW_TITLE, width, height);
    if (!m_windowManager->Initialize()) {
        LOG_ERROR("Failed to initialize window manager");
        return false;
    }
    LOG_INFO("Window manager initialized");

    // Set up window callbacks
    m_windowManager->SetFramebufferSizeCallback(
        [this](int width, int height) { this->OnFramebufferSizeChanged(width, height); });
    m_windowManager->SetKeyCallback(
        [this](int key, int scancode, int action, int mods) { this->OnKeyPressed(key, scancode, action, mods); });
    m_windowManager->SetWindowCloseCallback([this]() { this->OnWindowCloseRequested(); });

    // Create and initialize UI renderer
    m_uiRenderer = std::make_unique<UIRenderer>();
    if (!m_uiRenderer->Initialize(m_windowManager->GetNativeWindow())) {
        LOG_ERROR("Failed to initialize UI renderer");
        return false;
    }
    LOG_INFO("UI renderer initialized");

    // Initialize dialog manager
    m_dialogManager = std::make_unique<DialogManager>();
    LOG_INFO("Dialog manager initialized");

    // Initialize update checker
    m_updateChecker = std::make_unique<UpdateChecker>("andynicholson", "MetaImGUI");
    LOG_INFO("Update checker initialized");

    // Check for updates asynchronously
    CheckForUpdates();

    m_initialized = true;
    LOG_INFO("Application initialized successfully");
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

    LOG_INFO("Shutting down application...");

    // Save configuration before shutdown
    if (m_configManager && m_windowManager) {
        // Save window size
        int width, height;
        m_windowManager->GetWindowSize(width, height);
        m_configManager->SetWindowSize(width, height);
        LOG_INFO("Saving window size: {}x{}", width, height);

        // Save current language
        m_configManager->SetString("language", Localization::Instance().GetCurrentLanguage());

        if (m_configManager->Save()) {
            LOG_INFO("Configuration saved successfully");
        }
    }

    // Shutdown subsystems in reverse order of initialization
    m_updateChecker.reset();
    m_dialogManager.reset();
    m_uiRenderer.reset();
    m_windowManager.reset();
    m_configManager.reset();

    m_initialized = false;
    LOG_INFO("Application shut down successfully");

    // Shutdown logger last
    Logger::Instance().Shutdown();
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

    // Render exit confirmation dialog
    if (m_showExitDialog) {
        auto& loc = Localization::Instance();
        std::string title = loc.Tr("exit.title");
        std::string message = loc.Tr("exit.message");

        m_dialogManager->ShowConfirmation(title, message, [this](bool confirmed) {
            if (confirmed && m_windowManager) {
                m_windowManager->RequestClose();
            } else if (m_windowManager) {
                // User cancelled - make sure close flag is cleared
                m_windowManager->CancelClose();
            }
            m_showExitDialog = false;
        });
    }

    // Render dialogs
    if (m_dialogManager) {
        m_dialogManager->Render();
    }

    // End ImGui frame and render
    m_uiRenderer->EndFrame();

    // Present the frame
    m_windowManager->EndFrame();
}

// Event Handlers

void Application::OnWindowCloseRequested() {
    // Intercept window close button - cancel the close and show dialog
    if (m_windowManager) {
        m_windowManager->CancelClose();
    }
    m_showExitDialog = true;
}

void Application::OnExitRequested() {
    // Show exit confirmation dialog instead of closing immediately
    m_showExitDialog = true;
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

        LOG_INFO("Update available: v{} (current: v{})", updateInfo.latestVersion, updateInfo.currentVersion);
    } else {
        m_statusMessage = "Ready";
        LOG_INFO("No updates available (current version: v{})", updateInfo.currentVersion);
    }
}

} // namespace MetaImGUI
