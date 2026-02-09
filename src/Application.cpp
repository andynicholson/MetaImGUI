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

#include "Application.h"

#include "ConfigManager.h"
#include "DialogManager.h"
#include "ISSTracker.h"
#include "Localization.h"
#include "Logger.h"
#include "UIRenderer.h"
#include "UpdateChecker.h"
#include "WindowManager.h"
#include "version.h"

#include <GLFW/glfw3.h>
#include <curl/curl.h>
#include <imgui.h>

#include <cstdlib> // for std::getenv

#ifdef __APPLE__
#include <mach-o/dyld.h> // for _NSGetExecutablePath
#endif

namespace MetaImGUI {

Application::Application() : m_statusMessage("Ready") {}

Application::~Application() {
    Shutdown();
}

bool Application::Initialize() {
    if (m_initialized) {
        return true;
    }

    // Initialize logger first - use appropriate location per platform
#ifdef __APPLE__
    // macOS: Use ~/Library/Logs/MetaImGUI/ directory
    // NOLINTNEXTLINE(concurrency-mt-unsafe) - Safe: called during single-threaded initialization
    const char* home = std::getenv("HOME");
    const std::string logPath =
        (home != nullptr) ? std::string(home) + "/Library/Logs/MetaImGUI/metaimgui.log" : "metaimgui.log";
#elif defined(_WIN32)
    // Windows: Use LOCALAPPDATA directory
    // NOLINTNEXTLINE(concurrency-mt-unsafe) - Safe: called during single-threaded initialization
    const char* localAppData = std::getenv("LOCALAPPDATA");
    const std::string logPath = (localAppData != nullptr)
                                    ? std::string(localAppData) + "\\MetaImGUI\\logs\\metaimgui.log"
                                    : "logs\\metaimgui.log";
#else
    // Linux: Use ~/.local/share/MetaImGUI/logs/ directory
    // NOLINTNEXTLINE(concurrency-mt-unsafe) - Safe: called during single-threaded initialization
    const char* home = std::getenv("HOME");
    const std::string logPath =
        (home != nullptr) ? std::string(home) + "/.local/share/MetaImGUI/logs/metaimgui.log" : "logs/metaimgui.log";
#endif
    Logger::Instance().Initialize(logPath, LogLevel::Info);
    LOG_INFO("Initializing MetaImGUI v{}", Version::VERSION);

    // Initialize libcurl globally (thread-safe) before any CURL handles are created.
    // Auto-init via curl_easy_init() is NOT thread-safe when called concurrently.
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Load configuration
    m_configManager = std::make_unique<ConfigManager>();
    if (m_configManager->Load()) {
        LOG_INFO("Configuration loaded successfully");
    } else {
        LOG_INFO("Using default configuration");
    }

    // Load translations and set language from config
    // CRITICAL: translations.json MUST be present and valid
    // Try multiple locations for translations file (for different package formats)
    std::vector<std::string> translationPaths = {
        "resources/translations/translations.json", // Development/local build
    };

    // Check if running from AppImage (METAIMGUI_APPDIR set by custom AppRun)
    // NOLINTNEXTLINE(concurrency-mt-unsafe) - Safe: called during single-threaded initialization
    const char* appdir = std::getenv("METAIMGUI_APPDIR");
    if (appdir != nullptr) {
        const std::string appdir_path =
            std::string(appdir) + "/usr/share/MetaImGUI/resources/translations/translations.json";
        translationPaths.insert(translationPaths.begin(), appdir_path); // Try AppImage location first
    }

#ifdef __APPLE__
    // macOS bundle resources path - use executable path to find bundle location
    // When launched from Finder, CWD is not reliable, so we need absolute path
    char executablePath[1024];
    uint32_t size = sizeof(executablePath);
    if (_NSGetExecutablePath(executablePath, &size) == 0) {
        // executablePath is like: /Applications/MetaImGUI.app/Contents/MacOS/MetaImGUI
        std::string exePath(executablePath);
        // Go up to Contents directory and into Resources
        size_t macosPos = exePath.rfind("/MacOS/");
        if (macosPos != std::string::npos) {
            std::string bundleResourcePath =
                exePath.substr(0, macosPos) + "/Resources/resources/translations/translations.json";
            translationPaths.insert(translationPaths.begin(), bundleResourcePath);
            LOG_DEBUG("macOS bundle resource path: {}", bundleResourcePath);
        }
    }
    // Fallback: relative path (for terminal launch from MacOS directory)
    translationPaths.emplace_back("../Resources/resources/translations/translations.json");
    translationPaths.emplace_back("MetaImGUI.app/Contents/Resources/resources/translations/translations.json");
#endif

    // Add system installation paths
    translationPaths.emplace_back(
        "../share/MetaImGUI/resources/translations/translations.json"); // Installed (relative to bin)
    translationPaths.emplace_back(
        "/usr/share/MetaImGUI/resources/translations/translations.json"); // System-wide install
    translationPaths.emplace_back(
        "/usr/local/share/MetaImGUI/resources/translations/translations.json"); // Local install

    bool translationsLoaded = false;
    for (const auto& path : translationPaths) {
        if (Localization::Instance().LoadTranslations(path)) {
            translationsLoaded = true;
            break;
        }
    }

    if (!translationsLoaded) {
        LOG_ERROR("========================================");
        LOG_ERROR("CRITICAL: Failed to load translations!");
        LOG_ERROR("UI will show translation keys instead of actual text");
        LOG_ERROR("Tried the following locations:");
        for (const auto& path : translationPaths) {
            LOG_ERROR("  - {}", path);
        }
        LOG_ERROR("This is a PACKAGING ERROR - file is missing from bundle");
        LOG_ERROR("========================================");
    }

    const std::string language = m_configManager->GetString("language").value_or("en");
    Localization::Instance().SetLanguage(language);

    // Create and initialize window manager
    auto windowSize = m_configManager->GetWindowSize();
    const int width = windowSize ? windowSize->first : DEFAULT_WIDTH;
    const int height = windowSize ? windowSize->second : DEFAULT_HEIGHT;

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
    m_windowManager->SetContextLossCallback([this]() { return this->OnContextLoss(); });

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

    // Initialize ISS tracker
    m_issTracker = std::make_unique<ISSTracker>();
    LOG_INFO("ISS tracker initialized");

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
        int width = 0;
        int height = 0;
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
    if (m_issTracker) {
        m_issTracker->StopTracking();
    }
    m_issTracker.reset();
    m_updateChecker.reset();
    m_dialogManager.reset();
    m_uiRenderer.reset();
    m_windowManager.reset();
    m_configManager.reset();

    // Clean up libcurl global state (after all CURL users are destroyed)
    curl_global_cleanup();

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

    // Consume any pending update result (thread-safe handoff from worker thread)
    {
        const std::lock_guard<std::mutex> lock(m_updateResultMutex);
        if (m_pendingUpdateResult) {
            m_updateCheckInProgress = false;
            m_latestUpdateInfo = std::move(m_pendingUpdateResult);
            m_showUpdateNotification = true;

            if (m_latestUpdateInfo->updateAvailable) {
                m_statusMessage = "Update available: v" + m_latestUpdateInfo->latestVersion;
                LOG_INFO("Update available: v{} (current: v{})", m_latestUpdateInfo->latestVersion,
                         m_latestUpdateInfo->currentVersion);
            } else {
                m_statusMessage = "Ready";
                LOG_INFO("No updates available (current version: v{})", m_latestUpdateInfo->currentVersion);
            }
        }
    }

    // Get frame time for FPS calculation
    const ImGuiIO& io = ImGui::GetIO();
    m_lastFrameTime = io.Framerate;

    // Prepare window for rendering
    m_windowManager->BeginFrame();

    // Start ImGui frame
    m_uiRenderer->BeginFrame();

    // Create full-screen main window
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                          ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
                                          ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    if (ImGui::Begin("MetaImGUI Main", nullptr, window_flags)) {
        // Render menu bar
        m_uiRenderer->RenderMenuBar([this]() { this->OnExitRequested(); }, [this]() { this->OnToggleDemoWindow(); },
                                    [this]() { this->OnCheckUpdatesRequested(); },
                                    [this]() { this->OnShowAboutRequested(); }, m_showDemoWindow,
                                    [this]() { this->OnToggleISSTracker(); }, m_showISSTracker);

        // Render main window content
        m_uiRenderer->RenderMainWindow([this]() { this->OnShowAboutRequested(); },
                                       [this]() { m_showDemoWindow = true; },
                                       [this]() { this->OnShowInputDialogRequested(); });

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

    if (m_showISSTracker) {
        m_uiRenderer->RenderISSTrackerWindow(m_showISSTracker, m_issTracker.get());
    }

    // Render exit confirmation dialog
    // Consume m_showExitDialog immediately so ShowConfirmation() is called
    // exactly once. The dialog is then managed by DialogManager internally
    // until the user interacts with it.
    if (m_showExitDialog) {
        m_showExitDialog = false;

        auto& loc = Localization::Instance();
        const std::string title = loc.Tr("exit.title");
        const std::string message = loc.Tr("exit.message");

        m_dialogManager->ShowConfirmation(title, message, [this](bool confirmed) {
            if (confirmed && m_windowManager) {
                m_windowManager->RequestClose();
            } else if (m_windowManager) {
                // User cancelled - make sure close flag is cleared
                m_windowManager->CancelClose();
            }
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

void Application::OnShowInputDialogRequested() {
    if (m_dialogManager) {
        auto& loc = Localization::Instance();
        m_dialogManager->ShowInputDialog(loc.Tr("input_dialog.title"), loc.Tr("input_dialog.prompt"), "",
                                         [this](const std::string& result) {
                                             auto& loc = Localization::Instance();
                                             if (!result.empty()) {
                                                 m_statusMessage = loc.Tr("status.input_received") + " " + result;
                                                 LOG_INFO("User input: {}", result);
                                             } else {
                                                 m_statusMessage = loc.Tr("status.input_cancelled");
                                             }
                                         });
    }
}

void Application::OnToggleISSTracker() {
    m_showISSTracker = !m_showISSTracker;
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
                if ((mods & GLFW_MOD_CONTROL) != 0) {
                    OnShowAboutRequested();
                }
                break;
            case GLFW_KEY_F9:
                // DEBUG: Simulate context loss for testing
                if ((mods & GLFW_MOD_SHIFT) != 0) {
                    LOG_WARNING("DEBUG: User triggered context loss simulation via Shift+F9");
                    if (OnContextLoss()) {
                        m_statusMessage = "DEBUG: Context recovery successful";
                    } else {
                        m_statusMessage = "DEBUG: Context recovery failed";
                    }
                }
                break;
            default:
                // Ignore other keys
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
    // Thread-safe: store result for main thread to consume in Render()
    // This callback runs on the worker thread, so we must not write to
    // main-thread state directly. Instead, we store the result under a
    // mutex and let the main thread pick it up.
    const std::lock_guard<std::mutex> lock(m_updateResultMutex);
    m_pendingUpdateResult = std::make_unique<UpdateInfo>(updateInfo);
}

bool Application::OnContextLoss() {
    LOG_WARNING("Application handling context loss - attempting to recreate UI renderer");

    // Shutdown UI renderer (this destroys ImGui/ImPlot contexts)
    if (m_uiRenderer) {
        m_uiRenderer->Shutdown();
    }

    // Recreate UI renderer with new contexts
    if (!m_uiRenderer->Initialize(m_windowManager->GetNativeWindow())) {
        LOG_ERROR("Failed to reinitialize UI renderer after context loss");
        m_statusMessage = "ERROR: Failed to recover from context loss";
        return false;
    }

    LOG_INFO("UI renderer successfully reinitialized after context loss");
    m_statusMessage = "Recovered from display context loss";
    return true;
}

} // namespace MetaImGUI
