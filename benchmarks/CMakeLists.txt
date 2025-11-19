# Benchmarks using Google Benchmark

# Find or fetch Google Benchmark
find_package(benchmark QUIET)

if(NOT benchmark_FOUND)
    message(STATUS "Google Benchmark not found, fetching from GitHub...")
    message(STATUS "Note: This requires git to be available in the build environment")

    include(FetchContent)

    # Use main branch which has better CMake support
    FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG main
        GIT_SHALLOW TRUE
    )

    # Benchmark options
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googlebenchmark)

    if(NOT TARGET benchmark::benchmark)
        message(WARNING "Failed to fetch Google Benchmark. Benchmarks will not be built.")
        message(WARNING "To fix: install benchmark library system-wide or ensure git is available")
        return()
    endif()
endif()

# Create benchmarks executable
add_executable(MetaImGUI_benchmarks
    benchmark_main.cpp
    benchmark_config.cpp
    benchmark_localization.cpp
    benchmark_logger.cpp
)

target_link_libraries(MetaImGUI_benchmarks PRIVATE
    benchmark::benchmark
    imgui
)

target_include_directories(MetaImGUI_benchmarks PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/json/include
)

# Link source files that benchmarks depend on
target_sources(MetaImGUI_benchmarks PRIVATE
    ${CMAKE_SOURCE_DIR}/src/ConfigManager.cpp
    ${CMAKE_SOURCE_DIR}/src/Localization.cpp
    ${CMAKE_SOURCE_DIR}/src/Logger.cpp
)

# Platform-specific linking
if(WIN32)
    target_link_libraries(MetaImGUI_benchmarks PRIVATE shlwapi)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(MetaImGUI_benchmarks PRIVATE pthread dl)
elseif(APPLE)
    target_link_libraries(MetaImGUI_benchmarks PRIVATE pthread)
endif()

# Add custom target to run benchmarks
add_custom_target(run_benchmarks
    COMMAND MetaImGUI_benchmarks --benchmark_out=benchmark_results.json --benchmark_out_format=json
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running benchmarks..."
    VERBATIM
)

message(STATUS "Benchmarks enabled. Build target: MetaImGUI_benchmarks")

