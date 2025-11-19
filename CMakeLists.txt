cmake_minimum_required(VERSION 3.16)

# Include custom CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Get version from git
include(GetGitVersion)
get_git_version()

project(MetaImGUI VERSION ${PROJECT_VERSION} LANGUAGES CXX)

# Include code coverage module
include(CodeCoverage)
setup_coverage_flags()

# Include sanitizers module
include(Sanitizers)

# Include static analysis module
include(StaticAnalysis)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    message(STATUS "Build type not specified, defaulting to Release")
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform-specific compiler settings
if(MSVC)
    add_compile_options(/W4 /WX)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    # Enable multi-processor compilation
    add_compile_options(/MP)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLFW
if(WIN32)
    # On Windows, try to find GLFW via vcpkg first
    find_package(glfw3 CONFIG QUIET)
    if(NOT glfw3_FOUND)
        find_package(glfw3 REQUIRED)
    endif()
    set(GLFW_LIBRARIES glfw)
else()
    # On Linux, use pkg-config
    find_package(glfw3 REQUIRED)
    set(GLFW_LIBRARIES glfw)
endif()

# Setup ImGui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
if(EXISTS ${IMGUI_DIR})
    file(GLOB IMGUI_SOURCES
        ${IMGUI_DIR}/*.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )

    add_library(imgui STATIC ${IMGUI_SOURCES})
    target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
    )
    target_link_libraries(imgui PUBLIC OpenGL::GL ${GLFW_LIBRARIES})

    # Disable warnings for ImGui (external library)
    if(MSVC)
        target_compile_options(imgui PRIVATE /w)  # Disable all warnings for MSVC
    else()
        target_compile_options(imgui PRIVATE -w)  # Disable all warnings for GCC/Clang
    endif()

    # Silence OpenGL deprecation warnings on macOS
    if(APPLE)
        target_compile_definitions(imgui PUBLIC GL_SILENCE_DEPRECATION)
    endif()
else()
    message(FATAL_ERROR "ImGui not found. Run setup_dependencies.sh first.")
endif()

# Setup nlohmann/json (header-only library)
set(JSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/json)
if(EXISTS ${JSON_DIR})
    message(STATUS "nlohmann/json found at ${JSON_DIR}")
else()
    message(WARNING "nlohmann/json not found. Run setup_dependencies.sh first.")
endif()

# Generate version header
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/version.h
    @ONLY
)

# Main executable
add_executable(MetaImGUI
    src/main.cpp
    src/Application.cpp
    src/WindowManager.cpp
    src/UIRenderer.cpp
    src/ThemeManager.cpp
    src/UpdateChecker.cpp
    src/ConfigManager.cpp
    src/Logger.cpp
    src/DialogManager.cpp
    src/Localization.cpp
)

target_include_directories(MetaImGUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${JSON_DIR}/include
)

# Link libraries
target_link_libraries(MetaImGUI PRIVATE
    imgui
    OpenGL::GL
    ${GLFW_LIBRARIES}
)

# Platform-specific linking
if(WIN32)
    # Windows-specific libraries
    find_package(CURL REQUIRED)
    target_link_libraries(MetaImGUI PRIVATE user32 gdi32 shell32 CURL::libcurl)
    target_compile_definitions(MetaImGUI PRIVATE HAS_CURL)
elseif(UNIX AND NOT APPLE)
    # Linux-specific libraries (curl for update checking)
    find_package(CURL QUIET)
    if(CURL_FOUND)
        target_link_libraries(MetaImGUI PRIVATE dl pthread ${CURL_LIBRARIES})
        target_include_directories(MetaImGUI PRIVATE ${CURL_INCLUDE_DIRS})
        target_compile_definitions(MetaImGUI PRIVATE HAS_CURL)
    else()
        message(WARNING "libcurl not found. Update checking will be disabled.")
        target_link_libraries(MetaImGUI PRIVATE dl pthread)
    endif()
elseif(APPLE)
    # macOS-specific libraries
    find_package(CURL QUIET)
    if(CURL_FOUND)
        target_link_libraries(MetaImGUI PRIVATE ${CURL_LIBRARIES})
        target_include_directories(MetaImGUI PRIVATE ${CURL_INCLUDE_DIRS})
        target_compile_definitions(MetaImGUI PRIVATE HAS_CURL)
    else()
        message(WARNING "libcurl not found. Update checking will be disabled.")
    endif()
    # Silence OpenGL deprecation warnings on macOS
    target_compile_definitions(MetaImGUI PRIVATE GL_SILENCE_DEPRECATION)
endif()

# Copy resources to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Set output directory for Windows
if(WIN32)
    set_target_properties(MetaImGUI PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/Debug
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/Release
    )
endif()

# Debug configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(MetaImGUI PRIVATE DEBUG)
endif()

# Enable folder grouping in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Group source files
source_group("Source Files" FILES
    src/main.cpp
    src/Application.cpp
    src/WindowManager.cpp
    src/UIRenderer.cpp
    src/ThemeManager.cpp
    src/UpdateChecker.cpp
)
source_group("Header Files" FILES
    include/Application.h
    include/WindowManager.h
    include/UIRenderer.h
    include/ThemeManager.h
    include/UpdateChecker.h
)

# ============================================================================
# Testing (optional)
# ============================================================================
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    # Check if Catch2 is available
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/catch2)
        enable_testing()

        # Add Catch2
        add_subdirectory(external/catch2)

        # Create test executable
        add_executable(MetaImGUI_tests
            tests/test_main.cpp
            tests/test_version.cpp
            tests/test_update_checker.cpp
            tests/test_theme_manager.cpp
            tests/test_config_manager.cpp
            tests/test_logger.cpp
            tests/test_window_manager.cpp
            src/UpdateChecker.cpp
            src/ThemeManager.cpp
            src/ConfigManager.cpp
            src/Logger.cpp
            src/WindowManager.cpp
        )

        target_include_directories(MetaImGUI_tests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_BINARY_DIR}/include
            ${JSON_DIR}/include
        )

        target_link_libraries(MetaImGUI_tests PRIVATE
            Catch2::Catch2
            imgui
        )

        # Platform-specific linking for tests
        if(WIN32)
            find_package(CURL REQUIRED)
            target_link_libraries(MetaImGUI_tests PRIVATE CURL::libcurl)
        elseif(UNIX AND NOT APPLE)
            if(CURL_FOUND)
                target_link_libraries(MetaImGUI_tests PRIVATE ${CURL_LIBRARIES})
                target_include_directories(MetaImGUI_tests PRIVATE ${CURL_INCLUDE_DIRS})
            endif()
        elseif(APPLE)
            if(CURL_FOUND)
                target_link_libraries(MetaImGUI_tests PRIVATE ${CURL_LIBRARIES})
                target_include_directories(MetaImGUI_tests PRIVATE ${CURL_INCLUDE_DIRS})
            endif()
        endif()

        # Register tests with CTest
        include(CTest)
        include(Catch)
        catch_discover_tests(MetaImGUI_tests)

        # Add coverage target if enabled
        add_coverage_target(MetaImGUI_tests)

        message(STATUS "Tests enabled. Run with: ctest --test-dir build")
    else()
        message(WARNING "Catch2 not found. Tests will not be built. Run setup_dependencies.sh first.")
    endif()
endif()

# ============================================================================
# Benchmarks (optional)
# ============================================================================
option(BUILD_BENCHMARKS "Build benchmarks" OFF)

if(BUILD_BENCHMARKS)
    message(STATUS "Benchmarks enabled")
    add_subdirectory(benchmarks)
endif()
