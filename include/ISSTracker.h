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
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>

namespace MetaImGUI {

// ISS position data structure
struct ISSPosition {
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0; // km
    double velocity = 0.0; // km/h
    long timestamp = 0;    // Unix timestamp
    bool valid = false;
};

/**
 * @brief ISS Tracker that fetches ISS position data asynchronously
 *
 * This class demonstrates:
 * - Properly threaded async JSON requests using std::jthread
 * - JSON decoding with nlohmann/json
 * - Thread-safe data access for ImGui/ImPlot rendering
 * - Circular buffer for historical position tracking
 */
class ISSTracker {
public:
    ISSTracker();
    ~ISSTracker();

    // Disable copy and move
    ISSTracker(const ISSTracker&) = delete;
    ISSTracker& operator=(const ISSTracker&) = delete;
    ISSTracker(ISSTracker&&) = delete;
    ISSTracker& operator=(ISSTracker&&) = delete;

    /**
     * @brief Start tracking ISS position asynchronously
     * @param callback Function to call when new position data is available
     */
    void StartTracking(std::function<void(const ISSPosition&)> callback = nullptr);

    /**
     * @brief Stop tracking
     */
    void StopTracking();

    /**
     * @brief Check if tracking is active
     */
    bool IsTracking() const;

    /**
     * @brief Get the current ISS position (thread-safe)
     */
    ISSPosition GetCurrentPosition() const;

    /**
     * @brief Get position history for orbit trail (thread-safe)
     * @param latitudes Output vector for latitude values
     * @param longitudes Output vector for longitude values
     */
    void GetPositionHistory(std::vector<double>& latitudes, std::vector<double>& longitudes) const;

    /**
     * @brief Get the maximum number of positions stored in history
     */
    size_t GetMaxHistorySize() const {
        return m_maxHistorySize;
    }

    /**
     * @brief Manually fetch ISS position once (synchronous)
     */
    ISSPosition FetchPositionSync();

private:
    // API endpoint
    static constexpr const char* ISS_API_URL = "https://api.wheretheiss.at/v1/satellites/25544";

    // Maximum number of historical positions to store
    static constexpr size_t m_maxHistorySize = 100;

    // Current position and history
    ISSPosition m_currentPosition;
    std::deque<ISSPosition> m_positionHistory;
    mutable std::mutex m_dataMutex;

    // Threading
    std::atomic<bool> m_tracking;
    std::jthread m_trackingThread;
    mutable std::mutex m_threadMutex;

    // Callback (protected by m_callbackMutex)
    std::function<void(const ISSPosition&)> m_callback;
    mutable std::mutex m_callbackMutex;

    // Internal methods
    void TrackingLoop(const std::stop_token& stopToken);
    ISSPosition FetchPositionImpl();
    std::string FetchJSON(const std::string& url);
    ISSPosition ParseJSON(const std::string& jsonResponse);
    void AddToHistory(const ISSPosition& position);
};

} // namespace MetaImGUI
