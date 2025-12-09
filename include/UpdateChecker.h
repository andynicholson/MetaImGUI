#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>

namespace MetaImGUI {

// C++20: Using designated initializers for clear, safe initialization
struct UpdateInfo {
    bool updateAvailable = false;
    std::string latestVersion;
    std::string currentVersion;
    std::string releaseUrl;
    std::string releaseNotes;
    std::string downloadUrl;
};

class UpdateChecker {
public:
    UpdateChecker(std::string repoOwner, std::string repoName);
    ~UpdateChecker();

    // Delete copy and move
    UpdateChecker(const UpdateChecker&) = delete;
    UpdateChecker& operator=(const UpdateChecker&) = delete;
    UpdateChecker(UpdateChecker&&) = delete;
    UpdateChecker& operator=(UpdateChecker&&) = delete;

    // Check for updates asynchronously
    void CheckForUpdatesAsync(std::function<void(const UpdateInfo&)> callback);

    // Check for updates synchronously (blocking)
    UpdateInfo CheckForUpdates();

    // Cancel ongoing check
    void Cancel();

    // Check if a check is in progress
    [[nodiscard]] bool IsChecking() const;

    // Compare versions (returns: -1 if v1 < v2, 0 if equal, 1 if v1 > v2)
    static int CompareVersions(const std::string& v1, const std::string& v2);

private:
    std::string m_repoOwner;
    std::string m_repoName;
    std::atomic<bool> m_checking;

    // C++20: Using std::jthread for automatic thread management
    std::jthread m_checkThread;
    std::stop_source m_stopSource;
    std::mutex m_threadMutex; // Protects thread operations

    // Internal implementation
    UpdateInfo CheckForUpdatesImpl(const std::stop_token& stopToken);
    std::string FetchLatestReleaseInfo();
    UpdateInfo ParseReleaseInfo(const std::string& jsonResponse);
};

} // namespace MetaImGUI
