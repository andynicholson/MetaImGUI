# GetGitVersion.cmake
# Extracts version information from git tags and commit hash

function(get_git_version)
    # Try to get version from git tag
    execute_process(
        COMMAND git describe --tags --abbrev=0
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # Try to get full git description (includes commits since tag)
    execute_process(
        COMMAND git describe --tags --always --dirty
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DESCRIBE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # Get commit hash
    execute_process(
        COMMAND git rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # Get branch name
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # Parse version from tag (assumes format vX.Y.Z)
    if(GIT_TAG)
        string(REGEX REPLACE "^v([0-9]+)\\.([0-9]+)\\.([0-9]+).*" "\\1" VERSION_MAJOR "${GIT_TAG}")
        string(REGEX REPLACE "^v([0-9]+)\\.([0-9]+)\\.([0-9]+).*" "\\2" VERSION_MINOR "${GIT_TAG}")
        string(REGEX REPLACE "^v([0-9]+)\\.([0-9]+)\\.([0-9]+).*" "\\3" VERSION_PATCH "${GIT_TAG}")

        # Check if we successfully parsed the version
        if(NOT VERSION_MAJOR MATCHES "^[0-9]+$")
            set(VERSION_MAJOR "1")
            set(VERSION_MINOR "0")
            set(VERSION_PATCH "0")
        endif()
    else()
        # Default version if no git tag found
        set(VERSION_MAJOR "1")
        set(VERSION_MINOR "0")
        set(VERSION_PATCH "0")
    endif()

    # Set build type if not set
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release")
    endif()

    # Set parent scope variables
    set(PROJECT_VERSION_MAJOR ${VERSION_MAJOR} PARENT_SCOPE)
    set(PROJECT_VERSION_MINOR ${VERSION_MINOR} PARENT_SCOPE)
    set(PROJECT_VERSION_PATCH ${VERSION_PATCH} PARENT_SCOPE)
    set(PROJECT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" PARENT_SCOPE)
    set(PROJECT_VERSION_FULL "${GIT_DESCRIBE}" PARENT_SCOPE)
    set(GIT_COMMIT_HASH "${GIT_COMMIT_HASH}" PARENT_SCOPE)
    set(GIT_BRANCH "${GIT_BRANCH}" PARENT_SCOPE)
    set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" PARENT_SCOPE)

    # Print version info
    message(STATUS "Project Version: ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    if(GIT_DESCRIBE)
        message(STATUS "Git Description: ${GIT_DESCRIBE}")
    endif()
    if(GIT_COMMIT_HASH)
        message(STATUS "Git Commit: ${GIT_COMMIT_HASH}")
    endif()
    if(GIT_BRANCH)
        message(STATUS "Git Branch: ${GIT_BRANCH}")
    endif()
endfunction()

