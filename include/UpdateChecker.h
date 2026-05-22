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
enum class UpdateCheckStatus {
    Unknown,      ///< No check has run, or result not yet available
    UpToDate,     ///< Current version is the latest
    UpdateFound,  ///< A newer release is available
    RateLimited,  ///< GitHub API rate limit hit (HTTP 403)
    NetworkError, ///< Network/HTTP failure
    ParseError,   ///< Response did not contain a parseable version
    Cancelled     ///< Stop requested before the check completed
};

struct UpdateInfo {
    bool updateAvailable = false;
    std::string latestVersion;
    std::string currentVersion;
    std::string releaseUrl;
    std::string releaseNotes;
    std::string downloadUrl;
    UpdateCheckStatus status = UpdateCheckStatus::Unknown;
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

    // m_checking is the single source of truth for "a check is running".
    // Atomic compare_exchange on this guards entry; the worker resets it on exit.
    std::atomic<bool> m_checking;

    // C++20: Using std::jthread for automatic thread management
    std::jthread m_checkThread;
    std::mutex m_threadMutex; // Serialises thread (re)creation against itself

    // Internal implementation
    UpdateInfo CheckForUpdatesImpl(const std::stop_token& stopToken);
    UpdateInfo ParseReleaseInfo(const std::string& jsonResponse);
};

} // namespace MetaImGUI
