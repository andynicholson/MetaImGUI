#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace MetaImGUI {

/**
 * @brief Types of message box buttons
 */
enum class MessageBoxButtons {
    OK,          ///< Single OK button
    OKCancel,    ///< OK and Cancel buttons
    YesNo,       ///< Yes and No buttons
    YesNoCancel, ///< Yes, No, and Cancel buttons
    RetryCancel  ///< Retry and Cancel buttons
};

/**
 * @brief Message box icons/types
 */
enum class MessageBoxIcon {
    Info,    ///< Information icon
    Warning, ///< Warning icon
    Error,   ///< Error icon
    Question ///< Question icon
};

/**
 * @brief Result from message box
 */
enum class MessageBoxResult { OK, Cancel, Yes, No, Retry, None };

/**
 * @brief Manager for common UI dialogs
 *
 * DialogManager provides reusable dialog components including
 * message boxes, input dialogs, and progress dialogs.
 */
class DialogManager {
public:
    DialogManager();
    ~DialogManager();

    // Prevent copying and moving
    DialogManager(const DialogManager&) = delete;
    DialogManager& operator=(const DialogManager&) = delete;
    DialogManager(DialogManager&&) = delete;
    DialogManager& operator=(DialogManager&&) = delete;

    /**
     * @brief Render all active dialogs
     * Call this each frame in your main render loop
     */
    void Render();

    // Message Box
    /**
     * @brief Show a message box
     * @param title Dialog title
     * @param message Message text
     * @param buttons Button configuration
     * @param icon Icon type
     * @param callback Function called with result (async)
     */
    void ShowMessageBox(const std::string& title, const std::string& message,
                        MessageBoxButtons buttons = MessageBoxButtons::OK, MessageBoxIcon icon = MessageBoxIcon::Info,
                        std::function<void(MessageBoxResult)> callback = nullptr);

    // Input Dialog
    /**
     * @brief Show an input dialog
     * @param title Dialog title
     * @param prompt Prompt text
     * @param defaultValue Default input value
     * @param callback Function called with input result (empty if cancelled)
     */
    void ShowInputDialog(const std::string& title, const std::string& prompt, const std::string& defaultValue = "",
                         std::function<void(const std::string&)> callback = nullptr);

    // Progress Dialog
    /**
     * @brief Show a progress dialog
     * @param title Dialog title
     * @param message Progress message
     * @return Dialog ID for updating progress
     */
    int ShowProgressDialog(const std::string& title, const std::string& message = "");

    /**
     * @brief Update progress dialog
     * @param dialogId Dialog ID from ShowProgressDialog
     * @param progress Progress value (0.0 to 1.0)
     * @param message Optional status message
     */
    void UpdateProgress(int dialogId, float progress, const std::string& message = "");

    /**
     * @brief Close progress dialog
     * @param dialogId Dialog ID from ShowProgressDialog
     */
    void CloseProgress(int dialogId);

    // List Selection Dialog
    /**
     * @brief Show a list selection dialog
     * @param title Dialog title
     * @param items List of items to choose from
     * @param callback Function called with selected index (-1 if cancelled)
     */
    void ShowListDialog(const std::string& title, const std::vector<std::string>& items,
                        std::function<void(int)> callback = nullptr);

    // Confirmation Dialog (convenience wrapper)
    /**
     * @brief Show a confirmation dialog (Yes/No)
     * @param title Dialog title
     * @param message Confirmation message
     * @param callback Function called with true if Yes, false if No
     */
    void ShowConfirmation(const std::string& title, const std::string& message,
                          std::function<void(bool)> callback = nullptr);

    /**
     * @brief Check if any dialog is currently open
     */
    [[nodiscard]] bool HasOpenDialog() const;

    /**
     * @brief Close all dialogs
     */
    void CloseAll();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    void RenderMessageBox();
    void RenderInputDialog();
    void RenderProgressDialogs();
    void RenderListDialog();
};

} // namespace MetaImGUI
