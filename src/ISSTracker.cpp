#include "ISSTracker.h"

#include "Logger.h"

#include <nlohmann/json.hpp>

#include <chrono>
#include <thread>

// HTTP requests using libcurl (cross-platform)
#include <curl/curl.h>

namespace MetaImGUI {

ISSTracker::ISSTracker() : m_tracking(false) {}

ISSTracker::~ISSTracker() {
    StopTracking();
}

void ISSTracker::StartTracking(std::function<void(const ISSPosition&)> callback) {
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_tracking) {
        LOG_INFO("ISS Tracker: Already tracking, skipping");
        return;
    }

    m_tracking = true;

    // Store callback under lock protection
    {
        std::lock_guard<std::mutex> callbackLock(m_callbackMutex);
        m_callback = callback;
    }

    // C++20: std::jthread with stop_token for clean cancellation
    m_stopSource = std::stop_source();
    m_trackingThread = std::jthread([this](std::stop_token stopToken) { TrackingLoop(stopToken); });

    LOG_INFO("ISS Tracker: Started tracking");
}

void ISSTracker::StopTracking() {
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (!m_tracking) {
        return;
    }

    // Request stop using stop_source
    m_stopSource.request_stop();
    m_tracking = false;

    LOG_INFO("ISS Tracker: Stopped tracking");
}

bool ISSTracker::IsTracking() const {
    return m_tracking;
}

ISSPosition ISSTracker::GetCurrentPosition() const {
    std::lock_guard<std::mutex> lock(m_dataMutex);
    return m_currentPosition;
}

void ISSTracker::GetPositionHistory(std::vector<double>& latitudes, std::vector<double>& longitudes) const {
    std::lock_guard<std::mutex> lock(m_dataMutex);

    latitudes.clear();
    longitudes.clear();
    latitudes.reserve(m_positionHistory.size());
    longitudes.reserve(m_positionHistory.size());

    for (const auto& pos : m_positionHistory) {
        if (pos.valid) {
            latitudes.push_back(pos.latitude);
            longitudes.push_back(pos.longitude);
        }
    }
}

ISSPosition ISSTracker::FetchPositionSync() {
    return FetchPositionImpl();
}

void ISSTracker::TrackingLoop(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        try {
            ISSPosition position = FetchPositionImpl();

            // Check if stop was requested after fetch (important for slow networks)
            if (stopToken.stop_requested()) {
                LOG_INFO("ISS Tracker: Stop requested, discarding fetched data");
                break;
            }

            if (position.valid) {
                // Update current position and add to history
                {
                    std::lock_guard<std::mutex> lock(m_dataMutex);
                    m_currentPosition = position;
                    AddToHistory(position);
                }

                // Invoke callback if set (copy under lock to avoid data race)
                std::function<void(const ISSPosition&)> callback;
                {
                    std::lock_guard<std::mutex> lock(m_callbackMutex);
                    callback = m_callback;
                }

                if (callback) {
                    try {
                        callback(position);
                    } catch (const std::exception& e) {
                        LOG_ERROR("ISS Tracker: Callback threw exception: {}", e.what());
                    } catch (...) {
                        LOG_ERROR("ISS Tracker: Callback threw unknown exception");
                    }
                }

                LOG_INFO("ISS Tracker: Position updated - Lat: {}, Long: {}, Alt: {} km, Vel: {} km/h",
                         position.latitude, position.longitude, position.altitude, position.velocity);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("ISS Tracker: Error fetching position: {}", e.what());
        } catch (...) {
            LOG_ERROR("ISS Tracker: Unknown error fetching position");
        }

        // Wait 5 seconds before next update (if not stopped)
        auto start = std::chrono::steady_clock::now();
        while (!stopToken.stop_requested()) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
            if (elapsed >= 5) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    LOG_INFO("ISS Tracker: Tracking loop exited");
}

ISSPosition ISSTracker::FetchPositionImpl() {
    ISSPosition position;
    position.valid = false;

    try {
        std::string jsonResponse = FetchJSON(ISS_API_URL);
        if (jsonResponse.empty()) {
            LOG_ERROR("ISS Tracker: Empty response from server");
            return position;
        }

        position = ParseJSON(jsonResponse);
    } catch (const std::bad_alloc& e) {
        LOG_ERROR("ISS Tracker: Memory allocation failed: {}", e.what());
    } catch (const std::exception& e) {
        LOG_ERROR("ISS Tracker: Fetch failed: {}", e.what());
    } catch (...) {
        LOG_ERROR("ISS Tracker: Unknown error during fetch");
    }

    return position;
}

// Callback for libcurl to write response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string ISSTracker::FetchJSON(const std::string& url) {
    std::string result;

    CURL* curl = curl_easy_init();
    if (!curl) {
        LOG_ERROR("ISS Tracker: Failed to initialize CURL");
        return result;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "MetaImGUI-ISSTracker/1.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L); // Increased to 30 seconds for slow networks
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        LOG_ERROR("ISS Tracker: Request failed: {}", curl_easy_strerror(res));
        result.clear();
    }

    curl_easy_cleanup(curl);

    return result;
}

ISSPosition ISSTracker::ParseJSON(const std::string& jsonResponse) {
    ISSPosition position;
    position.valid = false;

    try {
        auto json = nlohmann::json::parse(jsonResponse);

        // Parse required fields
        if (json.contains("latitude") && json.contains("longitude")) {
            position.latitude = json["latitude"].get<double>();
            position.longitude = json["longitude"].get<double>();

            // Parse optional fields
            if (json.contains("altitude")) {
                position.altitude = json["altitude"].get<double>();
            }
            if (json.contains("velocity")) {
                position.velocity = json["velocity"].get<double>();
            }
            if (json.contains("timestamp")) {
                position.timestamp = json["timestamp"].get<long>();
            }

            position.valid = true;
        } else {
            LOG_ERROR("ISS Tracker: Missing required fields in JSON response");
        }
    } catch (const nlohmann::json::parse_error& e) {
        LOG_ERROR("ISS Tracker: JSON parse error: {}", e.what());
    } catch (const nlohmann::json::type_error& e) {
        LOG_ERROR("ISS Tracker: JSON type error: {}", e.what());
    } catch (const std::exception& e) {
        LOG_ERROR("ISS Tracker: Error parsing JSON: {}", e.what());
    }

    return position;
}

void ISSTracker::AddToHistory(const ISSPosition& position) {
    // Note: Caller must hold m_dataMutex

    if (!position.valid) {
        return;
    }

    // Add to history
    m_positionHistory.push_back(position);

    // Keep only the last maxHistorySize positions
    while (m_positionHistory.size() > m_maxHistorySize) {
        m_positionHistory.pop_front();
    }
}

} // namespace MetaImGUI
