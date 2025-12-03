# ISS Tracker Implementation Summary

## Overview
This document describes the ISS Tracker feature that was added to MetaImGUI as a complete example of properly threaded async JSON requests, data decoding, and ImPlot visualization.

## What Was Implemented

### 1. ImPlot Integration
- **Added ImPlot library** to `external/implot/`
- **Updated CMakeLists.txt** to build and link ImPlot
- **Updated setup scripts** (`setup_dependencies.sh` and `setup_dependencies.bat`) to automatically download ImPlot v0.16

### 2. ISSTracker Class
**Files:** `include/ISSTracker.h` and `src/ISSTracker.cpp`

#### Key Features:
- **Thread Management**: Uses C++20 `std::jthread` with `std::stop_token` for clean cancellation
- **Async HTTP Requests**: Uses libcurl to fetch data from `https://api.wheretheiss.at/v1/satellites/25544`
- **JSON Parsing**: Uses nlohmann/json to decode ISS position data (latitude, longitude, altitude, velocity, timestamp)
- **Thread Safety**: Mutex-protected data access between worker thread and UI thread
- **Circular Buffer**: Stores up to 100 historical positions for orbit trail visualization
- **Callback Support**: Optional callback when new position data is received

#### API:
```cpp
// Start/stop tracking (fetches every 5 seconds)
void StartTracking(std::function<void(const ISSPosition&)> callback = nullptr);
void StopTracking();

// Get current position (thread-safe)
ISSPosition GetCurrentPosition() const;

// Get position history for plotting
void GetPositionHistory(std::vector<double>& latitudes, std::vector<double>& longitudes) const;

// Manual one-time fetch
ISSPosition FetchPositionSync();
```

### 3. ISS Tracker Window with ImPlot
**File:** `src/UIRenderer.cpp` - `RenderISSTrackerWindow()` method

#### Features:
- **Control Panel**: Start/Stop tracking, manual fetch button
- **Status Display**: Shows current latitude, longitude, altitude, velocity, and timestamp
- **ImPlot Visualization**:
  - World map coordinate system (-180 to 180° longitude, -90 to 90° latitude)
  - Orbit trail showing historical positions (line plot)
  - Current position marker (large red circle)
  - Reference lines for Equator and Prime Meridian
  - Custom styling and markers

### 4. Application Integration
**Files:** `include/Application.h` and `src/Application.cpp`

#### Integration Points:
- **Initialization**: ISSTracker instance created during `Application::Initialize()`
- **Shutdown**: Properly stops tracking and cleans up in `Application::Shutdown()`
- **Menu Item**: Added "ISS Tracker" option in View menu
- **Rendering**: ISS tracker window rendered in main render loop
- **State Management**: Window visibility toggle via `m_showISSTracker`

## Usage

### Building the Project
```bash
# Download dependencies (including ImPlot)
./setup_dependencies.sh

# Build
cd build
cmake ..
make -j$(nproc)

# Run
./MetaImGUI
```

### Using the ISS Tracker
1. Launch MetaImGUI
2. Go to **View → ISS Tracker** menu
3. Click **Start Tracking** to begin fetching ISS position every 5 seconds
4. Click **Fetch Now** for an immediate one-time update
5. The plot shows:
   - **Red marker**: Current ISS position
   - **Blue line**: Orbit trail (last 100 positions)
   - **Gray lines**: Equator and Prime Meridian references

## Technical Highlights

### Proper Threading Pattern
```cpp
// Worker thread with stop_token support
m_trackingThread = std::jthread([this](std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        ISSPosition position = FetchPositionImpl();
        // Update data with mutex protection
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            m_currentPosition = position;
            AddToHistory(position);
        }
        // Wait 5 seconds (with cancellation support)
        // ...
    }
});
```

### JSON Decoding
```cpp
auto json = nlohmann::json::parse(jsonResponse);
position.latitude = json["latitude"].get<double>();
position.longitude = json["longitude"].get<double>();
position.altitude = json["altitude"].get<double>();
position.velocity = json["velocity"].get<double>();
position.timestamp = json["timestamp"].get<long>();
```

### ImPlot Usage
```cpp
if (ImPlot::BeginPlot("ISS Orbit", ImVec2(-1, -1))) {
    ImPlot::SetupAxes("Longitude (°)", "Latitude (°)");
    ImPlot::SetupAxisLimits(ImAxis_X1, -180, 180, ImGuiCond_Always);
    ImPlot::SetupAxisLimits(ImAxis_Y1, -90, 90, ImGuiCond_Always);

    // Plot orbit trail
    ImPlot::PlotLine("Orbit Trail", longitudes.data(), latitudes.data(), count);

    // Plot current position
    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 10.0f, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
    ImPlot::PlotScatter("Current Position", &longitude, &latitude, 1);

    ImPlot::EndPlot();
}
```

## Files Created
- `include/ISSTracker.h` - Header for ISS tracker class
- `src/ISSTracker.cpp` - Implementation with async fetch and JSON parsing
- `ISS_TRACKER_IMPLEMENTATION.md` - This documentation

## Files Modified
- `CMakeLists.txt` - Added ImPlot library and ISSTracker.cpp to build
- `include/Application.h` - Added ISSTracker member and handler
- `src/Application.cpp` - Initialize, shutdown, and render ISS tracker
- `include/UIRenderer.h` - Added RenderISSTrackerWindow declaration
- `src/UIRenderer.cpp` - Implemented ISS tracker window with ImPlot, added menu item
- `setup_dependencies.sh` - Download ImPlot
- `setup_dependencies.bat` - Download ImPlot

## API Response Example
The ISS tracking API returns JSON like this:
```json
{
  "name": "iss",
  "id": 25544,
  "latitude": 45.8376,
  "longitude": -122.6789,
  "altitude": 418.23,
  "velocity": 27560.45,
  "visibility": "daylight",
  "footprint": 4527.85,
  "timestamp": 1733270400,
  "daynum": 2460646.5,
  "solar_lat": -22.3456,
  "solar_lon": 180.1234,
  "units": "kilometers"
}
```

## Benefits of This Implementation

1. **Production-Ready Threading**: Uses modern C++20 features for safe thread management
2. **Non-Blocking UI**: All network operations happen on background thread
3. **Thread-Safe Data Access**: Proper mutex protection prevents race conditions
4. **Clean Resource Management**: Automatic cleanup via RAII and smart pointers
5. **Error Handling**: Comprehensive try-catch blocks and logging
6. **User-Friendly**: Simple controls with visual feedback
7. **Educational Value**: Demonstrates best practices for async operations in ImGui applications

## Future Enhancements (Optional)
- Add map background image overlay
- Show multiple satellites
- Add orbit prediction lines
- Export tracking data to CSV
- Add notification when ISS passes over specific location
- Configurable refresh interval

