# Benchmarks using Google Benchmark

# Try to find Google Benchmark (prefer system installation)
find_package(benchmark QUIET)

if(NOT benchmark_FOUND)
    message(STATUS "Google Benchmark not found locally, trying pkg-config...")
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(BENCHMARK benchmark)
        if(BENCHMARK_FOUND)
            add_library(benchmark::benchmark INTERFACE IMPORTED)
            target_link_libraries(benchmark::benchmark INTERFACE ${BENCHMARK_LIBRARIES})
            target_include_directories(benchmark::benchmark INTERFACE ${BENCHMARK_INCLUDE_DIRS})
            message(STATUS "Found Google Benchmark via pkg-config")
        endif()
    endif()
endif()

if(NOT benchmark_FOUND AND NOT BENCHMARK_FOUND)
    message(STATUS "Google Benchmark not found via CMake or pkg-config, fetching from GitHub...")
    message(STATUS "Note: Installing libbenchmark-dev is recommended to avoid build issues")

    include(FetchContent)

    # Use v1.7.1 - last known stable version without CMake parsing issues
    FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.7.1
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
        message(WARNING "To fix: sudo apt-get install libbenchmark-dev")
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

