#include "UIRenderer.h"

#include "ThemeManager.h"
#include "UpdateChecker.h"
#include "version.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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

UIRenderer::UIRenderer() : m_initialized(false) {}

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

void UIRenderer::RenderMainWindow(std::function<void()> onShowAbout, std::function<void()> onShowDemo) {
    // Calculate heights
    const float contentHeight = ImGui::GetContentRegionAvail().y - UILayout::STATUS_BAR_HEIGHT;

    // Main content area
    if (ImGui::BeginChild("MainContent", ImVec2(0, contentHeight), false, 0)) {
        ImGui::SetCursorPos(ImVec2(UILayout::LEFT_MARGIN, UILayout::TOP_MARGIN));
        ImGui::Text("Welcome to MetaImGUI!");

        ImGui::SetCursorPos(ImVec2(UILayout::LEFT_MARGIN, UILayout::TOP_MARGIN + UILayout::LINE_SPACING));
        ImGui::Text("This is a template for creating ImGui-based applications.");

        ImGui::SetCursorPos(ImVec2(UILayout::LEFT_MARGIN, UILayout::TOP_MARGIN + UILayout::LINE_SPACING * 2));
        ImGui::Text("Use the menu bar above to access the About dialog.");

        ImGui::SetCursorPos(ImVec2(UILayout::LEFT_MARGIN,
                                   UILayout::TOP_MARGIN + UILayout::LINE_SPACING * 2 + UILayout::BUTTON_SPACING));
        if (ImGui::Button("Show About Dialog")) {
            if (onShowAbout) {
                onShowAbout();
            }
        }

        ImGui::SetCursorPos(ImVec2(UILayout::LEFT_MARGIN,
                                   UILayout::TOP_MARGIN + UILayout::LINE_SPACING * 2 + UILayout::BUTTON_SPACING * 2));
        if (ImGui::Button("Show ImGui Demo Window")) {
            if (onShowDemo) {
                onShowDemo();
            }
        }
    }
    ImGui::EndChild();
}

void UIRenderer::RenderMenuBar(std::function<void()> onExit, std::function<void()> onToggleDemo,
                               std::function<void()> onCheckUpdates, std::function<void()> onShowAbout,
                               bool showDemoWindow) {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                if (onExit) {
                    onExit();
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("ImGui Demo Window", nullptr, showDemoWindow)) {
                if (onToggleDemo) {
                    onToggleDemo();
                }
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Theme")) {
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
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Check for Updates...")) {
                if (onCheckUpdates) {
                    onCheckUpdates();
                }
            }
            ImGui::Separator();
            if (ImGui::MenuItem("About", "Ctrl+A")) {
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
    ImVec4 windowBg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    ImVec4 statusBarBg = ImVec4(windowBg.x * 0.85f, // Slightly darker/lighter than window
                                windowBg.y * 0.85f, windowBg.z * 0.85f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, statusBarBg);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(UILayout::WINDOW_PADDING_X, UILayout::WINDOW_PADDING_Y));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(UILayout::ITEM_SPACING_X, UILayout::ITEM_SPACING_Y));

    // Draw subtle top separator line - theme-aware
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 statusBarPos = ImGui::GetCursorScreenPos();
    ImVec2 lineEnd = ImVec2(statusBarPos.x + ImGui::GetContentRegionAvail().x, statusBarPos.y);
    ImVec4 separatorColor = ImGui::GetStyle().Colors[ImGuiCol_Separator];
    drawList->AddLine(statusBarPos, lineEnd, ImGui::ColorConvertFloat4ToU32(separatorColor), 1.0f);

    if (ImGui::BeginChild("StatusBar", ImVec2(0, UILayout::STATUS_BAR_HEIGHT), false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        // Left side - Status message with indicator

        // Reserve space for the circle and get aligned position
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + UILayout::STATUS_CIRCLE_RADIUS * 2 +
                             UILayout::STATUS_CIRCLE_PADDING);
        ImGui::AlignTextToFramePadding();

        // Draw status indicator circle aligned with text
        drawList = ImGui::GetWindowDrawList();
        ImVec2 textPos = ImGui::GetCursorScreenPos();
        ImVec2 circleCenter = ImVec2(textPos.x - UILayout::STATUS_CIRCLE_RADIUS - UILayout::STATUS_CIRCLE_PADDING / 2,
                                     textPos.y + ImGui::GetFrameHeight() * 0.5f);

        ImU32 indicatorColor;
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

    ImGui::SetNextWindowSize(ImVec2(UILayout::ABOUT_WINDOW_WIDTH, UILayout::ABOUT_WINDOW_HEIGHT),
                             ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowFocus();
    if (ImGui::Begin("About MetaImGUI", &showAboutWindow, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("MetaImGUI v%s", Version::VERSION);
        ImGui::TextDisabled("Build: %s", Version::VERSION_FULL);
        ImGui::Separator();

        ImGui::Text("A template for creating ImGui-based applications");
        ImGui::Spacing();

        ImGui::Text("Built with:");
        ImGui::BulletText("ImGui v1.92.4");
        ImGui::BulletText("GLFW");
        ImGui::BulletText("OpenGL 3.3");
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
        if (ImGui::Button("Close")) {
            showAboutWindow = false;
        }
    }
    ImGui::End();
}

void UIRenderer::RenderUpdateNotification(bool& showUpdateNotification, UpdateInfo* updateInfo) {
    if (!updateInfo) {
        showUpdateNotification = false;
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(UILayout::UPDATE_WINDOW_WIDTH, UILayout::UPDATE_WINDOW_HEIGHT),
                             ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::Begin("Update Available", &showUpdateNotification, ImGuiWindowFlags_NoCollapse)) {
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
            ImGui::BeginChild("ReleaseNotes", ImVec2(0, UILayout::RELEASE_NOTES_HEIGHT), true);
            ImGui::TextWrapped("%s", updateInfo->releaseNotes.c_str());
            ImGui::EndChild();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Visit the release page to download:");
        ImGui::Spacing();

        // Buttons
        if (ImGui::Button("Open Release Page", ImVec2(UILayout::BUTTON_OPEN_RELEASE_WIDTH, UILayout::BUTTON_HEIGHT))) {
            // Open URL in browser
            std::string url = updateInfo->releaseUrl;
#ifdef _WIN32
            std::string cmd = "start " + url;
#elif __APPLE__
            std::string cmd = "open " + url;
#else
            std::string cmd = "xdg-open " + url;
#endif
            // Explicitly ignore return value - we're just launching a browser
            [[maybe_unused]] int result = system(cmd.c_str());
        }

        ImGui::SameLine();

        if (ImGui::Button("Remind Me Later", ImVec2(UILayout::BUTTON_REMIND_LATER_WIDTH, UILayout::BUTTON_HEIGHT))) {
            showUpdateNotification = false;
        }

        ImGui::SameLine();

        if (ImGui::Button("Close", ImVec2(UILayout::BUTTON_CLOSE_WIDTH, UILayout::BUTTON_HEIGHT))) {
            showUpdateNotification = false;
        }
    }
    ImGui::End();
}

void UIRenderer::ShowDemoWindow(bool& showDemoWindow) {
    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
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
