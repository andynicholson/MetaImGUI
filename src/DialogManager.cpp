#include "DialogManager.h"

#include "Localization.h"

#include <imgui.h>

#include <algorithm>
#include <map>

namespace MetaImGUI {

// Internal dialog state structures
struct MessageBoxState {
    std::string title;
    std::string message;
    MessageBoxButtons buttons;
    MessageBoxIcon icon;
    std::function<void(MessageBoxResult)> callback;
    bool open = true;
};

struct InputDialogState {
    std::string title;
    std::string prompt;
    char inputBuffer[256] = {};
    std::function<void(const std::string&)> callback;
    bool open = true;
};

struct ProgressDialogState {
    int id;
    std::string title;
    std::string message;
    float progress = 0.0f;
    bool open = true;
};

struct ListDialogState {
    std::string title;
    std::vector<std::string> items;
    int selectedIndex = -1;
    std::function<void(int)> callback;
    bool open = true;
};

struct DialogManager::Impl {
    std::unique_ptr<MessageBoxState> messageBox;
    std::unique_ptr<InputDialogState> inputDialog;
    std::unique_ptr<ListDialogState> listDialog;
    std::map<int, ProgressDialogState> progressDialogs;
    int nextProgressId = 1;
};

DialogManager::DialogManager() : m_impl(std::make_unique<Impl>()) {}

DialogManager::~DialogManager() = default;

void DialogManager::Render() {
    if (m_impl->messageBox && m_impl->messageBox->open) {
        RenderMessageBox();
    }

    if (m_impl->inputDialog && m_impl->inputDialog->open) {
        RenderInputDialog();
    }

    if (!m_impl->progressDialogs.empty()) {
        RenderProgressDialogs();
    }

    if (m_impl->listDialog && m_impl->listDialog->open) {
        RenderListDialog();
    }
}

void DialogManager::ShowMessageBox(const std::string& title, const std::string& message, MessageBoxButtons buttons,
                                   MessageBoxIcon icon, std::function<void(MessageBoxResult)> callback) {
    m_impl->messageBox = std::make_unique<MessageBoxState>();
    m_impl->messageBox->title = title;
    m_impl->messageBox->message = message;
    m_impl->messageBox->buttons = buttons;
    m_impl->messageBox->icon = icon;
    m_impl->messageBox->callback = callback;
    m_impl->messageBox->open = true;
}

void DialogManager::ShowInputDialog(const std::string& title, const std::string& prompt,
                                    const std::string& defaultValue, std::function<void(const std::string&)> callback) {
    m_impl->inputDialog = std::make_unique<InputDialogState>();
    m_impl->inputDialog->title = title;
    m_impl->inputDialog->prompt = prompt;
    m_impl->inputDialog->callback = callback;
    m_impl->inputDialog->open = true;

    // Copy default value to buffer
    strncpy(m_impl->inputDialog->inputBuffer, defaultValue.c_str(), sizeof(m_impl->inputDialog->inputBuffer) - 1);
    m_impl->inputDialog->inputBuffer[sizeof(m_impl->inputDialog->inputBuffer) - 1] = '\0';
}

int DialogManager::ShowProgressDialog(const std::string& title, const std::string& message) {
    int id = m_impl->nextProgressId++;
    ProgressDialogState state;
    state.id = id;
    state.title = title;
    state.message = message;
    state.progress = 0.0f;
    state.open = true;
    m_impl->progressDialogs[id] = state;
    return id;
}

void DialogManager::UpdateProgress(int dialogId, float progress, const std::string& message) {
    auto it = m_impl->progressDialogs.find(dialogId);
    if (it != m_impl->progressDialogs.end()) {
        it->second.progress = std::clamp(progress, 0.0f, 1.0f);
        if (!message.empty()) {
            it->second.message = message;
        }
    }
}

void DialogManager::CloseProgress(int dialogId) {
    m_impl->progressDialogs.erase(dialogId);
}

void DialogManager::ShowListDialog(const std::string& title, const std::vector<std::string>& items,
                                   std::function<void(int)> callback) {
    m_impl->listDialog = std::make_unique<ListDialogState>();
    m_impl->listDialog->title = title;
    m_impl->listDialog->items = items;
    m_impl->listDialog->selectedIndex = -1;
    m_impl->listDialog->callback = callback;
    m_impl->listDialog->open = true;
}

void DialogManager::ShowConfirmation(const std::string& title, const std::string& message,
                                     std::function<void(bool)> callback) {
    ShowMessageBox(title, message, MessageBoxButtons::YesNo, MessageBoxIcon::Question,
                   [callback](MessageBoxResult result) {
                       if (callback) {
                           callback(result == MessageBoxResult::Yes);
                       }
                   });
}

bool DialogManager::HasOpenDialog() const {
    return (m_impl->messageBox && m_impl->messageBox->open) || (m_impl->inputDialog && m_impl->inputDialog->open) ||
           !m_impl->progressDialogs.empty() || (m_impl->listDialog && m_impl->listDialog->open);
}

void DialogManager::CloseAll() {
    m_impl->messageBox.reset();
    m_impl->inputDialog.reset();
    m_impl->listDialog.reset();
    m_impl->progressDialogs.clear();
}

// Rendering implementations

void DialogManager::RenderMessageBox() {
    auto& mb = m_impl->messageBox;
    if (!mb || !mb->open)
        return;

    ImGui::OpenPopup(mb->title.c_str());
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    MessageBoxResult result = MessageBoxResult::None;

    if (ImGui::BeginPopupModal(mb->title.c_str(), &mb->open,
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        // Icon and message
        const char* iconText = "";
        ImVec4 iconColor = ImVec4(1, 1, 1, 1);
        switch (mb->icon) {
            case MessageBoxIcon::Info:
                iconText = "[i]";
                iconColor = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
                break;
            case MessageBoxIcon::Warning:
                iconText = "[!]";
                iconColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
                break;
            case MessageBoxIcon::Error:
                iconText = "[X]";
                iconColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
                break;
            case MessageBoxIcon::Question:
                iconText = "[?]";
                iconColor = ImVec4(0.4f, 0.8f, 0.4f, 1.0f);
                break;
        }

        ImGui::TextColored(iconColor, "%s", iconText);
        ImGui::SameLine();
        ImGui::TextWrapped("%s", mb->message.c_str());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Buttons (translated)
        auto& loc = Localization::Instance();
        float buttonWidth = 100.0f;
        switch (mb->buttons) {
            case MessageBoxButtons::OK:
                if (ImGui::Button(loc.Tr("button.ok").c_str(), ImVec2(buttonWidth, 0))) {
                    result = MessageBoxResult::OK;
                    mb->open = false;
                }
                break;

            case MessageBoxButtons::OKCancel:
                if (ImGui::Button(loc.Tr("button.ok").c_str(), ImVec2(buttonWidth, 0))) {
                    result = MessageBoxResult::OK;
                    mb->open = false;
                }
                ImGui::SameLine();
                if (ImGui::Button(loc.Tr("button.cancel").c_str(), ImVec2(buttonWidth, 0))) {
                    result = MessageBoxResult::Cancel;
                    mb->open = false;
                }
                break;

            case MessageBoxButtons::YesNo:
                if (ImGui::Button(loc.Tr("button.yes").c_str(), ImVec2(buttonWidth, 0))) {
                    result = MessageBoxResult::Yes;
                    mb->open = false;
                }
                ImGui::SameLine();
                if (ImGui::Button(loc.Tr("button.no").c_str(), ImVec2(buttonWidth, 0))) {
                    result = MessageBoxResult::No;
                    mb->open = false;
                }
                break;

            case MessageBoxButtons::YesNoCancel:
                if (ImGui::Button(loc.Tr("button.yes").c_str(), ImVec2(buttonWidth, 0))) {
                    result = MessageBoxResult::Yes;
                    mb->open = false;
                }
                ImGui::SameLine();
                if (ImGui::Button(loc.Tr("button.no").c_str(), ImVec2(buttonWidth, 0))) {
                    result = MessageBoxResult::No;
                    mb->open = false;
                }
                ImGui::SameLine();
                if (ImGui::Button(loc.Tr("button.cancel").c_str(), ImVec2(buttonWidth, 0))) {
                    result = MessageBoxResult::Cancel;
                    mb->open = false;
                }
                break;

            case MessageBoxButtons::RetryCancel:
                if (ImGui::Button(loc.Tr("button.retry").c_str(), ImVec2(buttonWidth, 0))) {
                    result = MessageBoxResult::Retry;
                    mb->open = false;
                }
                ImGui::SameLine();
                if (ImGui::Button(loc.Tr("button.cancel").c_str(), ImVec2(buttonWidth, 0))) {
                    result = MessageBoxResult::Cancel;
                    mb->open = false;
                }
                break;
        }

        ImGui::EndPopup();
    }

    // Handle close and callback
    if (!mb->open && result != MessageBoxResult::None) {
        if (mb->callback) {
            mb->callback(result);
        }
        m_impl->messageBox.reset();
    }
}

void DialogManager::RenderInputDialog() {
    auto& id = m_impl->inputDialog;
    if (!id || !id->open)
        return;

    ImGui::OpenPopup(id->title.c_str());
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    std::string result;
    bool submitted = false;

    if (ImGui::BeginPopupModal(id->title.c_str(), &id->open,
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Text("%s", id->prompt.c_str());
        ImGui::Spacing();

        ImGui::SetNextItemWidth(300.0f);
        if (ImGui::InputText("##input", id->inputBuffer, sizeof(id->inputBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
            submitted = true;
            result = id->inputBuffer;
            id->open = false;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        auto& loc = Localization::Instance();
        if (ImGui::Button(loc.Tr("button.ok").c_str(), ImVec2(100, 0))) {
            submitted = true;
            result = id->inputBuffer;
            id->open = false;
        }
        ImGui::SameLine();
        if (ImGui::Button(loc.Tr("button.cancel").c_str(), ImVec2(100, 0))) {
            submitted = false;
            id->open = false;
        }

        ImGui::EndPopup();
    }

    // Handle close and callback
    if (!id->open) {
        if (id->callback) {
            id->callback(submitted ? result : "");
        }
        m_impl->inputDialog.reset();
    }
}

void DialogManager::RenderProgressDialogs() {
    for (auto it = m_impl->progressDialogs.begin(); it != m_impl->progressDialogs.end();) {
        auto& pd = it->second;

        if (!pd.open) {
            it = m_impl->progressDialogs.erase(it);
            continue;
        }

        std::string popupId = pd.title + "##" + std::to_string(pd.id);
        ImGui::OpenPopup(popupId.c_str());
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal(popupId.c_str(), nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
            if (!pd.message.empty()) {
                ImGui::Text("%s", pd.message.c_str());
                ImGui::Spacing();
            }

            ImGui::ProgressBar(pd.progress, ImVec2(300, 0));
            ImGui::Spacing();

            // Show percentage
            ImGui::Text("%.1f%%", pd.progress * 100.0f);

            ImGui::EndPopup();
        }

        ++it;
    }
}

void DialogManager::RenderListDialog() {
    auto& ld = m_impl->listDialog;
    if (!ld || !ld->open)
        return;

    ImGui::OpenPopup(ld->title.c_str());
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    int selectedIndex = -1;
    bool confirmed = false;

    if (ImGui::BeginPopupModal(ld->title.c_str(), &ld->open,
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::BeginChild("ListBox", ImVec2(300, 200), ImGuiChildFlags_Border);

        for (size_t i = 0; i < ld->items.size(); ++i) {
            bool isSelected = (static_cast<int>(i) == ld->selectedIndex);
            if (ImGui::Selectable(ld->items[i].c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
                ld->selectedIndex = static_cast<int>(i);
                if (ImGui::IsMouseDoubleClicked(0)) {
                    confirmed = true;
                    selectedIndex = ld->selectedIndex;
                    ld->open = false;
                }
            }
        }

        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        auto& loc = Localization::Instance();
        if (ImGui::Button(loc.Tr("button.ok").c_str(), ImVec2(100, 0)) && ld->selectedIndex >= 0) {
            confirmed = true;
            selectedIndex = ld->selectedIndex;
            ld->open = false;
        }
        ImGui::SameLine();
        if (ImGui::Button(loc.Tr("button.cancel").c_str(), ImVec2(100, 0))) {
            ld->open = false;
        }

        ImGui::EndPopup();
    }

    // Handle close and callback
    if (!ld->open) {
        if (ld->callback) {
            ld->callback(confirmed ? selectedIndex : -1);
        }
        m_impl->listDialog.reset();
    }
}

} // namespace MetaImGUI
