#include "UpdateChecker.h"

#include "version.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

// HTTP requests using libcurl (cross-platform)
#include <curl/curl.h>

namespace MetaImGUI {

UpdateChecker::UpdateChecker(const std::string& repoOwner, const std::string& repoName)
    : m_repoOwner(repoOwner), m_repoName(repoName), m_checking(false) {}

UpdateChecker::~UpdateChecker() {
    // C++20: std::jthread automatically joins on destruction
    Cancel();
}

void UpdateChecker::CheckForUpdatesAsync(std::function<void(const UpdateInfo&)> callback) {
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_checking) {
        std::cout << "Update Checker: Check already in progress, skipping" << std::endl;
        return; // Already checking
    }

    m_checking = true;

    // C++20: std::jthread with stop_token for clean cancellation
    m_stopSource = std::stop_source();
    m_checkThread = std::jthread([this, callback](std::stop_token stopToken) {
        UpdateInfo info = CheckForUpdatesImpl(stopToken);

        m_checking = false;

        // Only invoke callback if not cancelled
        if (!stopToken.stop_requested() && callback) {
            try {
                callback(info);
            } catch (const std::exception& e) {
                std::cerr << "Update Checker: Callback threw exception: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Update Checker: Callback threw unknown exception" << std::endl;
            }
        }
    });
    // C++20: jthread automatically joins on destruction, no detach() needed
}

UpdateInfo UpdateChecker::CheckForUpdates() {
    m_checking = true;
    // For synchronous calls, use a default stop_token that never stops
    UpdateInfo info = CheckForUpdatesImpl(std::stop_token{});
    m_checking = false;
    return info;
}

void UpdateChecker::Cancel() {
    std::lock_guard<std::mutex> lock(m_threadMutex);

    // C++20: Request stop using stop_source
    m_stopSource.request_stop();

    // jthread automatically joins, so we just need to request stop
    std::cout << "Update Checker: Cancellation requested" << std::endl;
}

bool UpdateChecker::IsChecking() const {
    return m_checking;
}

UpdateInfo UpdateChecker::CheckForUpdatesImpl(std::stop_token stopToken) {
    // C++20: Using designated initializers for clear initialization
    UpdateInfo info{.updateAvailable = false,
                    .latestVersion = "",
                    .currentVersion = Version::VERSION,
                    .releaseUrl = "",
                    .releaseNotes = "",
                    .downloadUrl = ""};

    try {
        std::string jsonResponse = FetchLatestReleaseInfo();
        if (stopToken.stop_requested()) {
            std::cout << "Update Checker: Check cancelled by user" << std::endl;
            return info;
        }

        if (jsonResponse.empty()) {
            std::cerr << "Update Checker: Empty response from server" << std::endl;
            return info;
        }

        info = ParseReleaseInfo(jsonResponse);
        info.currentVersion = Version::VERSION;

        // Compare versions
        if (!info.latestVersion.empty()) {
            int cmp = CompareVersions(info.currentVersion, info.latestVersion);
            info.updateAvailable = (cmp < 0);

            if (info.updateAvailable) {
                std::cout << "Update Checker: Update available - " << info.currentVersion << " -> "
                          << info.latestVersion << std::endl;
            } else {
                std::cout << "Update Checker: No update available (current: " << info.currentVersion << ")"
                          << std::endl;
            }
        } else {
            std::cerr << "Update Checker: Could not parse latest version from response" << std::endl;
        }
    } catch (const std::bad_alloc& e) {
        std::cerr << "Update Checker: Memory allocation failed: " << e.what() << std::endl;
        info.updateAvailable = false;
    } catch (const std::exception& e) {
        std::cerr << "Update Checker: Check failed: " << e.what() << std::endl;
        info.updateAvailable = false;
    } catch (...) {
        std::cerr << "Update Checker: Unknown error during update check" << std::endl;
        info.updateAvailable = false;
    }

    return info;
}

// Unified cross-platform implementation using libcurl

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string UpdateChecker::FetchLatestReleaseInfo() {
    std::string result;

    CURL* curl = curl_easy_init();
    if (!curl) {
        return result;
    }

    std::string url = "https://api.github.com/repos/" + m_repoOwner + "/" + m_repoName + "/releases/latest";

    std::cout << "Update Checker: Requesting URL: " << url << std::endl;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "UpdateChecker/1.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cout << "Update Checker: Request failed: " << curl_easy_strerror(res) << std::endl;
        result.clear();
    }

    curl_easy_cleanup(curl);

    std::cout << "Update Checker: Response received (" << result.size() << " bytes):" << std::endl;
    std::cout << result << std::endl;

    return result;
}

UpdateInfo UpdateChecker::ParseReleaseInfo(const std::string& jsonResponse) {
    UpdateInfo info;

    if (jsonResponse.empty()) {
        std::cerr << "Update Checker: Empty JSON response" << std::endl;
        return info;
    }

    try {
        // Parse JSON using nlohmann/json library
        auto j = nlohmann::json::parse(jsonResponse);

        // Extract tag_name
        if (j.contains("tag_name") && j["tag_name"].is_string()) {
            std::string tag = j["tag_name"].get<std::string>();
            // Remove 'v' prefix if present
            info.latestVersion = (!tag.empty() && tag[0] == 'v') ? tag.substr(1) : tag;
            std::cout << "Update Checker: Parsed version: " << info.latestVersion << std::endl;
        } else {
            std::cerr << "Update Checker: No tag_name in response" << std::endl;
        }

        // Extract html_url
        if (j.contains("html_url") && j["html_url"].is_string()) {
            info.releaseUrl = j["html_url"].get<std::string>();
            std::cout << "Update Checker: Release URL: " << info.releaseUrl << std::endl;
        }

        // Extract body (release notes)
        if (j.contains("body") && j["body"].is_string()) {
            info.releaseNotes = j["body"].get<std::string>();
            std::cout << "Update Checker: Release notes: " << info.releaseNotes.length() << " chars" << std::endl;
        }

        // Extract download_url (if available)
        if (j.contains("assets") && j["assets"].is_array() && !j["assets"].empty()) {
            auto& firstAsset = j["assets"][0];
            if (firstAsset.contains("browser_download_url") && firstAsset["browser_download_url"].is_string()) {
                info.downloadUrl = firstAsset["browser_download_url"].get<std::string>();
            }
        }

    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Update Checker: JSON parse error: " << e.what() << std::endl;
        std::cerr << "Update Checker: Error at byte " << e.byte << std::endl;
    } catch (const nlohmann::json::type_error& e) {
        std::cerr << "Update Checker: JSON type error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Update Checker: Unexpected error parsing JSON: " << e.what() << std::endl;
    }

    return info;
}

int UpdateChecker::CompareVersions(const std::string& v1, const std::string& v2) {
    auto parseVersion = [](std::string version) {
        // Remove leading 'v' or 'V' if present
        if (!version.empty() && (version[0] == 'v' || version[0] == 'V')) {
            version = version.substr(1);
        }

        std::vector<int> parts;
        std::stringstream ss(version);
        std::string part;

        while (std::getline(ss, part, '.')) {
            // Extract numeric part only
            std::string numStr;
            for (char c : part) {
                if (std::isdigit(c)) {
                    numStr += c;
                } else {
                    break;
                }
            }
            if (!numStr.empty()) {
                parts.push_back(std::stoi(numStr));
            }
        }

        // Ensure at least 3 parts (major.minor.patch)
        while (parts.size() < 3) {
            parts.push_back(0);
        }

        return parts;
    };

    auto parts1 = parseVersion(v1);
    auto parts2 = parseVersion(v2);

    for (size_t i = 0; i < (std::min)(parts1.size(), parts2.size()); ++i) {
        if (parts1[i] < parts2[i])
            return -1;
        if (parts1[i] > parts2[i])
            return 1;
    }

    return 0;
}

} // namespace MetaImGUI
