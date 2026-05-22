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

#include "UpdateChecker.h"

#include "HttpClient.h"
#include "Logger.h"
#include "version.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string_view>

namespace MetaImGUI {

UpdateChecker::UpdateChecker(std::string repoOwner, std::string repoName)
    : m_repoOwner(std::move(repoOwner)), m_repoName(std::move(repoName)), m_checking(false) {}

UpdateChecker::~UpdateChecker() {
    // C++20: std::jthread requests stop and joins automatically.
    // Cancel() also requests stop on whatever thread is current.
    Cancel();
}

void UpdateChecker::CheckForUpdatesAsync(std::function<void(const UpdateInfo&)> callback) {
    // Single-source-of-truth gate: only one check may run at a time.
    // CAS keeps the gate honest without needing a mutex around the boolean.
    bool expected = false;
    if (!m_checking.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        LOG_INFO("Update Checker: Check already in progress, skipping");
        return;
    }

    // Serialise jthread (re)creation: assigning to a running jthread joins it,
    // and we don't want concurrent CheckForUpdatesAsync calls fighting over the slot.
    const std::lock_guard<std::mutex> lock(m_threadMutex);

    m_checkThread = std::jthread([this, callback](const std::stop_token& stopToken) {
        const UpdateInfo info = CheckForUpdatesImpl(stopToken);

        // Release the gate before invoking the callback so a callback that
        // re-triggers a check (e.g. user retry) doesn't deadlock against itself.
        m_checking.store(false, std::memory_order_release);

        if (!stopToken.stop_requested() && callback) {
            try {
                callback(info);
            } catch (const std::exception& e) {
                LOG_ERROR("Update Checker: Callback threw exception: {}", e.what());
            } catch (...) {
                LOG_ERROR("Update Checker: Callback threw unknown exception");
            }
        }
    });
}

UpdateInfo UpdateChecker::CheckForUpdates() {
    // Synchronous path: still respect the gate so async + sync don't run in parallel.
    bool expected = false;
    if (!m_checking.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        UpdateInfo info{};
        info.currentVersion = Version::VERSION;
        info.status = UpdateCheckStatus::Unknown;
        return info;
    }

    UpdateInfo info = CheckForUpdatesImpl(std::stop_token{});
    m_checking.store(false, std::memory_order_release);
    return info;
}

void UpdateChecker::Cancel() {
    // jthread tracks its own stop_source; request_stop() is idempotent and thread-safe.
    m_checkThread.request_stop();
    LOG_INFO("Update Checker: Cancellation requested");
}

bool UpdateChecker::IsChecking() const {
    return m_checking.load(std::memory_order_acquire);
}

UpdateInfo UpdateChecker::CheckForUpdatesImpl(const std::stop_token& stopToken) {
    UpdateInfo info{.updateAvailable = false,
                    .latestVersion = "",
                    .currentVersion = Version::VERSION,
                    .releaseUrl = "",
                    .releaseNotes = "",
                    .downloadUrl = "",
                    .status = UpdateCheckStatus::Unknown};

    try {
        const HttpClient http;
        const std::string url = "https://api.github.com/repos/" + m_repoOwner + "/" + m_repoName + "/releases/latest";
        LOG_INFO("Update Checker: Requesting URL: {}", url);

        // Two retries on transient network failure — GitHub occasionally
        // closes connections under load. Rate-limit and cancellation skip retry.
        const HttpRequest request{
            .url = url, .userAgent = "UpdateChecker/1.0", .timeout = std::chrono::seconds{10}, .maxRetries = 2};
        const HttpResponse response = http.Get(request, stopToken);

        switch (response.status) {
            case HttpStatus::Cancelled:
                LOG_INFO("Update Checker: Check cancelled by user");
                info.status = UpdateCheckStatus::Cancelled;
                return info;
            case HttpStatus::RateLimited:
                LOG_WARNING("Update Checker: GitHub API rate limit hit");
                info.status = UpdateCheckStatus::RateLimited;
                return info;
            case HttpStatus::NetworkError:
                LOG_ERROR("Update Checker: Network error fetching release info");
                info.status = UpdateCheckStatus::NetworkError;
                return info;
            case HttpStatus::Ok:
                break;
        }

        if (response.body.empty()) {
            LOG_ERROR("Update Checker: Empty response from server");
            info.status = UpdateCheckStatus::NetworkError;
            return info;
        }

        LOG_INFO("Update Checker: Response received ({} bytes)", response.body.size());
        info = ParseReleaseInfo(response.body);
        info.currentVersion = Version::VERSION;

        if (info.latestVersion.empty()) {
            LOG_ERROR("Update Checker: Could not parse latest version from response");
            info.status = UpdateCheckStatus::ParseError;
            return info;
        }

        const int cmp = CompareVersions(info.currentVersion, info.latestVersion);
        info.updateAvailable = (cmp < 0);
        info.status = info.updateAvailable ? UpdateCheckStatus::UpdateFound : UpdateCheckStatus::UpToDate;

        if (info.updateAvailable) {
            LOG_INFO("Update Checker: Update available - {} -> {}", info.currentVersion, info.latestVersion);
        } else {
            LOG_INFO("Update Checker: No update available (current: {})", info.currentVersion);
        }
    } catch (const std::bad_alloc& e) {
        LOG_ERROR("Update Checker: Memory allocation failed: {}", e.what());
        info.status = UpdateCheckStatus::NetworkError;
    } catch (const std::exception& e) {
        LOG_ERROR("Update Checker: Check failed: {}", e.what());
        info.status = UpdateCheckStatus::NetworkError;
    } catch (...) {
        LOG_ERROR("Update Checker: Unknown error during update check");
        info.status = UpdateCheckStatus::NetworkError;
    }

    return info;
}

UpdateInfo UpdateChecker::ParseReleaseInfo(const std::string& jsonResponse) {
    UpdateInfo info;

    if (jsonResponse.empty()) {
        LOG_ERROR("Update Checker: Empty JSON response");
        return info;
    }

    try {
        auto j = nlohmann::json::parse(jsonResponse);

        if (j.contains("tag_name") && j["tag_name"].is_string()) {
            std::string tag = j["tag_name"].get<std::string>();
            info.latestVersion = (!tag.empty() && tag[0] == 'v') ? tag.substr(1) : tag;
            LOG_INFO("Update Checker: Parsed version: {}", info.latestVersion);
        } else {
            LOG_ERROR("Update Checker: No tag_name in response");
        }

        if (j.contains("html_url") && j["html_url"].is_string()) {
            info.releaseUrl = j["html_url"].get<std::string>();
        }

        if (j.contains("body") && j["body"].is_string()) {
            info.releaseNotes = j["body"].get<std::string>();
        }

        if (j.contains("assets") && j["assets"].is_array() && !j["assets"].empty()) {
            auto& firstAsset = j["assets"][0];
            if (firstAsset.contains("browser_download_url") && firstAsset["browser_download_url"].is_string()) {
                info.downloadUrl = firstAsset["browser_download_url"].get<std::string>();
            }
        }

    } catch (const nlohmann::json::parse_error& e) {
        LOG_ERROR("Update Checker: JSON parse error: {} at byte {}", e.what(), e.byte);
    } catch (const nlohmann::json::type_error& e) {
        LOG_ERROR("Update Checker: JSON type error: {}", e.what());
    } catch (const std::exception& e) {
        LOG_ERROR("Update Checker: Unexpected error parsing JSON: {}", e.what());
    }

    return info;
}

namespace {

// SemVer 2.0 ordering of pre-release identifiers (see semver.org §11):
//  - A pre-release version has lower precedence than the release version
//    (1.2.0-rc1 < 1.2.0).
//  - Identifiers consisting of only digits are compared numerically;
//    alphanumerics are compared lexically; numeric < alphanumeric.
//  - A larger set of identifiers has higher precedence than a smaller set
//    that matches as a prefix.
struct ParsedVersion {
    std::vector<int> core;               // major.minor.patch (zero-padded to 3)
    std::vector<std::string> preRelease; // empty == release
};

bool IsAllDigits(const std::string& s) {
    if (s.empty()) {
        return false;
    }
    return std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
}

ParsedVersion ParseSemVer(std::string version) {
    ParsedVersion parsed;

    if (!version.empty() && (version[0] == 'v' || version[0] == 'V')) {
        version = version.substr(1);
    }

    // Strip build metadata ("+...") — it does not participate in precedence.
    const size_t plus = version.find('+');
    if (plus != std::string::npos) {
        version = version.substr(0, plus);
    }

    std::string corePart = version;
    std::string preReleasePart;
    const size_t dash = version.find('-');
    if (dash != std::string::npos) {
        corePart = version.substr(0, dash);
        preReleasePart = version.substr(dash + 1);
    }

    {
        std::stringstream ss(corePart);
        std::string part;
        while (std::getline(ss, part, '.')) {
            std::string digits;
            for (const char c : part) {
                if (std::isdigit(static_cast<unsigned char>(c)) != 0) {
                    digits += c;
                } else {
                    break;
                }
            }
            if (!digits.empty()) {
                parsed.core.push_back(std::stoi(digits));
            }
        }
    }
    while (parsed.core.size() < 3) {
        parsed.core.push_back(0);
    }

    if (!preReleasePart.empty()) {
        std::stringstream ss(preReleasePart);
        std::string id;
        while (std::getline(ss, id, '.')) {
            parsed.preRelease.push_back(id);
        }
    }

    return parsed;
}

int CompareIdentifiers(const std::string& a, const std::string& b) {
    const bool aDigits = IsAllDigits(a);
    const bool bDigits = IsAllDigits(b);
    if (aDigits && bDigits) {
        const int ai = std::stoi(a);
        const int bi = std::stoi(b);
        if (ai < bi) {
            return -1;
        }
        if (ai > bi) {
            return 1;
        }
        return 0;
    }
    // Numeric identifiers always have lower precedence than alphanumerics.
    if (aDigits && !bDigits) {
        return -1;
    }
    if (!aDigits && bDigits) {
        return 1;
    }
    return a.compare(b);
}

} // namespace

int UpdateChecker::CompareVersions(const std::string& v1, const std::string& v2) {
    const ParsedVersion p1 = ParseSemVer(v1);
    const ParsedVersion p2 = ParseSemVer(v2);

    for (size_t i = 0; i < std::min(p1.core.size(), p2.core.size()); ++i) {
        if (p1.core[i] < p2.core[i]) {
            return -1;
        }
        if (p1.core[i] > p2.core[i]) {
            return 1;
        }
    }

    // Cores equal — pre-release ranks below release.
    const bool p1Pre = !p1.preRelease.empty();
    const bool p2Pre = !p2.preRelease.empty();
    if (!p1Pre && !p2Pre) {
        return 0;
    }
    if (p1Pre && !p2Pre) {
        return -1;
    }
    if (!p1Pre && p2Pre) {
        return 1;
    }

    const size_t n = std::min(p1.preRelease.size(), p2.preRelease.size());
    for (size_t i = 0; i < n; ++i) {
        const int c = CompareIdentifiers(p1.preRelease[i], p2.preRelease[i]);
        if (c != 0) {
            return c;
        }
    }
    if (p1.preRelease.size() < p2.preRelease.size()) {
        return -1;
    }
    if (p1.preRelease.size() > p2.preRelease.size()) {
        return 1;
    }
    return 0;
}

} // namespace MetaImGUI
