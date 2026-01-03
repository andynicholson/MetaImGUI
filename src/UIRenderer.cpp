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

#include "UIRenderer.h"

#include "ISSTracker.h"
#include "Localization.h"
#include "Logger.h"
#include "ThemeManager.h"
#include "UpdateChecker.h"
#include "version.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#include <array>
#include <cstdlib>
#include <ctime>

namespace MetaImGUI {

// UI Layout Constants
namespace UILayout {
// Margins and spacing
constexpr float LEFT_MARGIN = 50.0f;
constexpr float TOP_MARGIN = 100.0f;
constexpr float LINE_SPACING = 30.0f;
constexpr float BUTTON_SPACING = 40.0f;
constexpr float STATUS_BAR_HEIGHT = 28.0f;

// Window sizes
constexpr float ABOUT_WINDOW_WIDTH = 450.0f;
constexpr float ABOUT_WINDOW_HEIGHT = 350.0f;
constexpr float UPDATE_WINDOW_WIDTH = 450.0f;
constexpr float UPDATE_WINDOW_HEIGHT = 300.0f;
constexpr float RELEASE_NOTES_HEIGHT = 120.0f;

// Button sizes
constexpr float BUTTON_OPEN_RELEASE_WIDTH = 200.0f;
constexpr float BUTTON_REMIND_LATER_WIDTH = 150.0f;
constexpr float BUTTON_CLOSE_WIDTH = 75.0f;
constexpr float BUTTON_HEIGHT = 30.0f;

// Status bar
constexpr float STATUS_CIRCLE_RADIUS = 5.0f;
constexpr float STATUS_CIRCLE_PADDING = 6.0f;
constexpr float STATUS_RIGHT_SIDE_WIDTH = 200.0f;

// Padding and style
constexpr float WINDOW_PADDING_X = 8.0f;
constexpr float WINDOW_PADDING_Y = 4.0f;
constexpr float ITEM_SPACING_X = 12.0f;
constexpr float ITEM_SPACING_Y = 0.0f;
constexpr float VERTICAL_SPACING_SMALL = 10.0f;
constexpr float TEXT_WRAP_POS_MULTIPLIER = 35.0f;
} // namespace UILayout

UIRenderer::UIRenderer() = default;

UIRenderer::~UIRenderer() {
    Shutdown();
}

bool UIRenderer::Initialize(GLFWwindow* window) {
    if (m_initialized) {
        return true;
    }

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup ImPlot context
    ImPlot::CreateContext();

    // Setup ImGui style
    ThemeManager::Apply(ThemeManager::Theme::Modern);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    m_initialized = true;
    return true;
}

void UIRenderer::Shutdown() {
    if (m_initialized) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        m_initialized = false;
    }
}

void UIRenderer::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UIRenderer::EndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIRenderer::RenderMainWindow(std::function<void()> onShowAbout, std::function<void()> onShowDemo,
                                  std::function<void()> onShowInputDialog) {
    auto& loc = Localization::Instance();

    // Calculate heights
    const float contentHeight = ImGui::GetContentRegionAvail().y - UILayout::STATUS_BAR_HEIGHT;

    // Main content area
    if (ImGui::BeginChild("MainContent", ImVec2(0, contentHeight), ImGuiChildFlags_None, ImGuiWindowFlags_None)) {
        ImGui::SetCursorPos(ImVec2(UILayout::LEFT_MARGIN, UILayout::TOP_MARGIN));
        ImGui::Text("Welcome to MetaImGUI!");

        ImGui::SetCursorPos(ImVec2(UILayout::LEFT_MARGIN, UILayout::TOP_MARGIN + UILayout::LINE_SPACING));
        ImGui::Text("This is a template for creating ImGui-based applications.");

        ImGui::SetCursorPos(ImVec2(UILayout::LEFT_MARGIN, UILayout::TOP_MARGIN + (UILayout::LINE_SPACING * 2)));
        ImGui::Text("Use the menu bar above to access the About dialog.");

        ImGui::SetCursorPos(ImVec2(UILayout::LEFT_MARGIN,
                                   UILayout::TOP_MARGIN + (UILayout::LINE_SPACING * 2) + UILayout::BUTTON_SPACING));
        if (ImGui::Button(loc.Tr("button.show_about").c_str())) {
            if (onShowAbout) {
                onShowAbout();
            }
        }

        ImGui::SetCursorPos(ImVec2(UILayout::LEFT_MARGIN, UILayout::TOP_MARGIN + (UILayout::LINE_SPACING * 2) +
                                                              (UILayout::BUTTON_SPACING * 2)));
        if (ImGui::Button(loc.Tr("button.show_demo").c_str())) {
            if (onShowDemo) {
                onShowDemo();
            }
        }

        ImGui::SetCursorPos(ImVec2(UILayout::LEFT_MARGIN, UILayout::TOP_MARGIN + (UILayout::LINE_SPACING * 2) +
                                                              (UILayout::BUTTON_SPACING * 3)));
        if (ImGui::Button(loc.Tr("button.show_input").c_str())) {
            if (onShowInputDialog) {
                onShowInputDialog();
            }
        }
    }
    ImGui::EndChild();
}

void UIRenderer::RenderMenuBar(std::function<void()> onExit, std::function<void()> onToggleDemo,
                               std::function<void()> onCheckUpdates, std::function<void()> onShowAbout,
                               bool showDemoWindow, std::function<void()> onToggleISSTracker, bool showISSTracker) {
    auto& loc = Localization::Instance();

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu(loc.Tr("menu.file").c_str())) {
            if (ImGui::MenuItem(loc.Tr("menu.exit").c_str(), "Alt+F4")) {
                if (onExit) {
                    onExit();
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(loc.Tr("menu.view").c_str())) {
            if (ImGui::MenuItem(loc.Tr("menu.demo_window").c_str(), nullptr, showDemoWindow)) {
                if (onToggleDemo) {
                    onToggleDemo();
                }
            }

            if (ImGui::MenuItem("ISS Tracker", nullptr, showISSTracker)) {
                if (onToggleISSTracker) {
                    onToggleISSTracker();
                }
            }

            ImGui::Separator();

            if (ImGui::BeginMenu(loc.Tr("menu.theme").c_str())) {
                if (ImGui::MenuItem("Dark", nullptr, ThemeManager::GetCurrent() == ThemeManager::Theme::Dark)) {
                    ThemeManager::Apply(ThemeManager::Theme::Dark);
                }
                if (ImGui::MenuItem("Light", nullptr, ThemeManager::GetCurrent() == ThemeManager::Theme::Light)) {
                    ThemeManager::Apply(ThemeManager::Theme::Light);
                }
                if (ImGui::MenuItem("Classic", nullptr, ThemeManager::GetCurrent() == ThemeManager::Theme::Classic)) {
                    ThemeManager::Apply(ThemeManager::Theme::Classic);
                }
                if (ImGui::MenuItem("Modern", nullptr, ThemeManager::GetCurrent() == ThemeManager::Theme::Modern)) {
                    ThemeManager::Apply(ThemeManager::Theme::Modern);
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::BeginMenu(loc.Tr("menu.language").c_str())) {
                if (ImGui::MenuItem("English", nullptr, loc.GetCurrentLanguage() == "en")) {
                    loc.SetLanguage("en");
                }
                if (ImGui::MenuItem("Español", nullptr, loc.GetCurrentLanguage() == "es")) {
                    loc.SetLanguage("es");
                }
                if (ImGui::MenuItem("Français", nullptr, loc.GetCurrentLanguage() == "fr")) {
                    loc.SetLanguage("fr");
                }
                if (ImGui::MenuItem("Deutsch", nullptr, loc.GetCurrentLanguage() == "de")) {
                    loc.SetLanguage("de");
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(loc.Tr("menu.help").c_str())) {
            if (ImGui::MenuItem(loc.Tr("menu.check_updates").c_str())) {
                if (onCheckUpdates) {
                    onCheckUpdates();
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem(loc.Tr("menu.about").c_str(), "Ctrl+A")) {
                if (onShowAbout) {
                    onShowAbout();
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void UIRenderer::RenderStatusBar(const std::string& statusMessage, float fps, const char* version,
                                 bool updateInProgress) {
    // Status bar styling - theme-aware background
    const ImVec4 windowBg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    const ImVec4 statusBarBg = ImVec4(windowBg.x * 0.85f, // Slightly darker/lighter than window
                                      windowBg.y * 0.85f, windowBg.z * 0.85f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, statusBarBg);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(UILayout::WINDOW_PADDING_X, UILayout::WINDOW_PADDING_Y));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(UILayout::ITEM_SPACING_X, UILayout::ITEM_SPACING_Y));

    // Draw subtle top separator line - theme-aware
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 statusBarPos = ImGui::GetCursorScreenPos();
    const ImVec2 lineEnd = ImVec2(statusBarPos.x + ImGui::GetContentRegionAvail().x, statusBarPos.y);
    const ImVec4 separatorColor = ImGui::GetStyle().Colors[ImGuiCol_Separator];
    drawList->AddLine(statusBarPos, lineEnd, ImGui::ColorConvertFloat4ToU32(separatorColor), 1.0f);

    if (ImGui::BeginChild("StatusBar", ImVec2(0, UILayout::STATUS_BAR_HEIGHT), ImGuiChildFlags_None,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        // Left side - Status message with indicator

        // Reserve space for the circle and get aligned position
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (UILayout::STATUS_CIRCLE_RADIUS * 2) +
                             UILayout::STATUS_CIRCLE_PADDING);
        ImGui::AlignTextToFramePadding();

        // Draw status indicator circle aligned with text
        drawList = ImGui::GetWindowDrawList();
        const ImVec2 textPos = ImGui::GetCursorScreenPos();
        const ImVec2 circleCenter =
            ImVec2(textPos.x - UILayout::STATUS_CIRCLE_RADIUS - (UILayout::STATUS_CIRCLE_PADDING / 2),
                   textPos.y + (ImGui::GetFrameHeight() * 0.5f));

        ImU32 indicatorColor = IM_COL32(50, 200, 50, 255); // Green by default
        if (updateInProgress) {
            indicatorColor = IM_COL32(255, 200, 50, 255); // Yellow for in-progress
        } else {
            indicatorColor = IM_COL32(50, 200, 50, 255); // Green for ready
        }

        drawList->AddCircleFilled(circleCenter, UILayout::STATUS_CIRCLE_RADIUS, indicatorColor);

        // Draw text
        ImGui::Text("%s", statusMessage.c_str());

        // Right side - Version and FPS
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - UILayout::STATUS_RIGHT_SIDE_WIDTH);

        // Version
        ImGui::TextDisabled("v%s", version);

        // Separator
        ImGui::SameLine();
        ImGui::TextDisabled("|");

        // FPS counter
        ImGui::SameLine();
        ImGui::TextDisabled("%.0f FPS", fps);
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void UIRenderer::RenderAboutWindow(bool& showAboutWindow) {
    if (!showAboutWindow) {
        return;
    }

    auto& loc = Localization::Instance();

    ImGui::SetNextWindowSize(ImVec2(UILayout::ABOUT_WINDOW_WIDTH, UILayout::ABOUT_WINDOW_HEIGHT),
                             ImGuiCond_FirstUseEver);
    if (ImGui::Begin("About MetaImGUI", &showAboutWindow, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("MetaImGUI v%s", Version::VERSION);
        ImGui::TextDisabled("Build: %s", Version::VERSION_FULL);
        ImGui::Separator();

        ImGui::Text("A template for creating ImGui-based applications");
        ImGui::Spacing();

        ImGui::Text("Built with:");
        ImGui::BulletText("ImGui v1.92.4");
        ImGui::BulletText("ImPlot v0.17");
        ImGui::BulletText("GLFW");
        ImGui::BulletText("OpenGL 4.6 (4.1 on macOS)");
        ImGui::BulletText("C++20");
        ImGui::Separator();

        ImGui::Text("This template provides:");
        ImGui::BulletText("Basic ImGui application structure");
        ImGui::BulletText("Cross-platform build system");
        ImGui::BulletText("Dependency management");
        ImGui::BulletText("Automated CI/CD and releases");
        ImGui::BulletText("Version management from git");
        ImGui::BulletText("Modern C++20 codebase");
        ImGui::Spacing();
        ImGui::TextWrapped("Use this as a starting point for your own ImGui applications!");

        ImGui::Separator();
        ImGui::TextDisabled("Git: %s (%s)", Version::COMMIT, Version::BRANCH);
        ImGui::TextDisabled("Config: %s", Version::BUILD_CONFIG);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + UILayout::VERTICAL_SPACING_SMALL);
        if (ImGui::Button(loc.Tr("button.close").c_str())) {
            showAboutWindow = false;
        }
    }
    ImGui::End();
}

void UIRenderer::RenderUpdateNotification(bool& showUpdateNotification, UpdateInfo* updateInfo) {
    if (updateInfo == nullptr) {
        showUpdateNotification = false;
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(UILayout::UPDATE_WINDOW_WIDTH, UILayout::UPDATE_WINDOW_HEIGHT),
                             ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    const char* windowTitle = updateInfo->updateAvailable ? "Update Available" : "No Updates Available";
    if (ImGui::Begin(windowTitle, &showUpdateNotification, ImGuiWindowFlags_NoCollapse)) {
        if (updateInfo->updateAvailable) {
            // Update available UI
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
            ImGui::Text("A new version is available!");
            ImGui::PopStyleColor();

            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Current version: v%s", updateInfo->currentVersion.c_str());
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
            ImGui::Text("Latest version:  v%s", updateInfo->latestVersion.c_str());
            ImGui::PopStyleColor();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (!updateInfo->releaseNotes.empty()) {
                ImGui::Text("Release Notes:");
                ImGui::BeginChild("ReleaseNotes", ImVec2(0, UILayout::RELEASE_NOTES_HEIGHT), ImGuiChildFlags_Border);
                ImGui::TextWrapped("%s", updateInfo->releaseNotes.c_str());
                ImGui::EndChild();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Visit the release page to download:");
            ImGui::Spacing();

            // Buttons for update available
            if (ImGui::Button("Open Release Page",
                              ImVec2(UILayout::BUTTON_OPEN_RELEASE_WIDTH, UILayout::BUTTON_HEIGHT))) {
                // Open URL in browser
                const std::string url = updateInfo->releaseUrl;

                // Security: Validate URL before opening to prevent command injection
                // Only allow https:// URLs from github.com
                bool isValidUrl = false;
                if (url.starts_with("https://")) {
                    // Check if it contains github.com (the expected domain)
                    if (url.find("github.com") != std::string::npos) {
                        // Check for shell metacharacters that could be used for injection
                        const std::string dangerousChars = ";|&$`\n<>(){}[]'\"\\";
                        bool hasDangerousChars = false;
                        for (const char c : dangerousChars) {
                            if (url.find(c) != std::string::npos) {
                                hasDangerousChars = true;
                                break;
                            }
                        }
                        isValidUrl = !hasDangerousChars;
                    }
                }

                if (isValidUrl) {
#ifdef _WIN32
                    // Windows: Use system() with start command
                    const std::string cmd = "start \"\" \"" + url + "\"";
                    [[maybe_unused]] const int result = std::system(cmd.c_str());
#elif __APPLE__
                    // macOS: Use system() but with validated/escaped URL
                    const std::string cmd = "open \"" + url + "\"";
                    [[maybe_unused]] const int result = std::system(cmd.c_str());
#else
                    // Linux: Use system() but with validated/escaped URL
                    const std::string cmd = "xdg-open \"" + url + "\"";
                    // NOLINTNEXTLINE(concurrency-mt-unsafe) - Safe: URL validated, single-threaded UI context
                    [[maybe_unused]] const int result = std::system(cmd.c_str());
#endif
                } else {
                    LOG_ERROR("Rejected potentially malicious URL: {}", url);
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Remind Me Later",
                              ImVec2(UILayout::BUTTON_REMIND_LATER_WIDTH, UILayout::BUTTON_HEIGHT))) {
                showUpdateNotification = false;
            }

            ImGui::SameLine();

            if (ImGui::Button("Close", ImVec2(UILayout::BUTTON_CLOSE_WIDTH, UILayout::BUTTON_HEIGHT))) {
                showUpdateNotification = false;
            }
        } else {
            // No update available UI
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 0.4f, 1.0f));
            ImGui::Text("You're up to date!");
            ImGui::PopStyleColor();

            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Current version: v%s", updateInfo->currentVersion.c_str());
            if (!updateInfo->latestVersion.empty()) {
                ImGui::Text("Latest version:  v%s", updateInfo->latestVersion.c_str());
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextWrapped("You are running the latest version of MetaImGUI.");
            ImGui::TextWrapped("Check back later for updates!");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Center the OK button
            const float buttonWidth = 100.0f;
            const float windowWidth = ImGui::GetWindowSize().x;
            ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);

            if (ImGui::Button("OK", ImVec2(buttonWidth, UILayout::BUTTON_HEIGHT))) {
                showUpdateNotification = false;
            }
        }
    }
    ImGui::End();
}

void UIRenderer::ShowDemoWindow(bool& showDemoWindow) {
    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }
}

void UIRenderer::RenderISSTrackerWindow(bool& showISSTracker, ISSTracker* issTracker) {
    if (!showISSTracker || issTracker == nullptr) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(900, 700), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("ISS Tracker", &showISSTracker)) {
        // Get current position
        const ISSPosition currentPos = issTracker->GetCurrentPosition();

        // Control panel
        ImGui::BeginGroup();
        {
            ImGui::Text("ISS Position Tracker");
            ImGui::Separator();

            // Control buttons
            if (issTracker->IsTracking()) {
                if (ImGui::Button("Stop Tracking")) {
                    issTracker->StopTracking();
                }
            } else {
                if (ImGui::Button("Start Tracking")) {
                    issTracker->StartTracking();
                }
            }

            ImGui::Separator();

            // Display current position info
            if (currentPos.valid) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 0.4f, 1.0f));
                ImGui::Text("Status: Active");
                ImGui::PopStyleColor();

                ImGui::Spacing();
                ImGui::Text("Latitude:  %.4f°", currentPos.latitude);
                ImGui::Text("Longitude: %.4f°", currentPos.longitude);
                ImGui::Text("Altitude:  %.2f km", currentPos.altitude);
                ImGui::Text("Velocity:  %.2f km/h", currentPos.velocity);

                // Convert Unix timestamp to readable time
                if (currentPos.timestamp > 0) {
                    const auto time = static_cast<time_t>(currentPos.timestamp);
                    struct tm timeinfo = {};
                    std::array<char, 80> buffer{};

                    // Use thread-safe versions of gmtime
#ifdef _WIN32
                    if (gmtime_s(&timeinfo, &time) == 0) {
                        strftime(buffer.data(), buffer.size(), "%Y-%m-%d %H:%M:%S UTC", &timeinfo);
                        ImGui::Text("Time: %s", buffer.data());
                    } else {
                        ImGui::Text("Time: (error converting timestamp)");
                    }
#else
                    if (gmtime_r(&time, &timeinfo) != nullptr) {
                        strftime(buffer.data(), buffer.size(), "%Y-%m-%d %H:%M:%S UTC", &timeinfo);
                        ImGui::Text("Time: %s", buffer.data());
                    } else {
                        ImGui::Text("Time: (error converting timestamp)");
                    }
#endif
                }
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.4f, 0.4f, 1.0f));
                ImGui::Text("Status: No data");
                ImGui::PopStyleColor();
                ImGui::TextWrapped("Click 'Start Tracking' or 'Fetch Now' to get ISS position data.");
            }
        }
        ImGui::EndGroup();

        ImGui::Separator();

        // Get position history
        std::vector<double> latitudes, longitudes;
        issTracker->GetPositionHistory(latitudes, longitudes);

        // Plot area
        if (ImPlot::BeginPlot("ISS Orbit", ImVec2(-1, -1))) {
            // Set axis limits for Earth coordinates
            ImPlot::SetupAxes("Longitude (°)", "Latitude (°)");
            ImPlot::SetupAxisLimits(ImAxis_X1, -180, 180, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, -90, 90, ImGuiCond_Always);

            // Plot orbit trail if we have data
            if (!latitudes.empty() && !longitudes.empty()) {
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 2.0f);
                ImPlot::PlotLine("Orbit Trail", longitudes.data(), latitudes.data(),
                                 static_cast<int>(longitudes.size()));
            }

            // Plot current position as a larger marker
            if (currentPos.valid) {
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 5.0f, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                ImPlot::PlotScatter("Current Position", &currentPos.longitude, &currentPos.latitude, 1);
            }

            // Add grid reference lines
            ImPlot::SetNextLineStyle(ImVec4(0.5f, 0.5f, 0.5f, 0.3f));
            std::array<double, 2> xRange = {-180.0, 180.0};
            std::array<double, 2> yZero = {0.0, 0.0};
            ImPlot::PlotLine("Equator", xRange.data(), yZero.data(), 2);

            std::array<double, 2> xZero = {0.0, 0.0};
            std::array<double, 2> yRange = {-90.0, 90.0};
            ImPlot::PlotLine("Prime Meridian", xZero.data(), yRange.data(), 2);

            ImPlot::EndPlot();
        }
    }
    ImGui::End();

    // If window was closed, stop tracking
    if (!showISSTracker && issTracker->IsTracking()) {
        issTracker->StopTracking();
    }
}

void UIRenderer::HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * UILayout::TEXT_WRAP_POS_MULTIPLIER);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

} // namespace MetaImGUI
