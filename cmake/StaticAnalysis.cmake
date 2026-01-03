# Static Analysis Configuration
# Integrates clang-tidy and cppcheck into the build process

option(ENABLE_CLANG_TIDY "Enable clang-tidy during build" OFF)
option(ENABLE_CPPCHECK "Enable cppcheck during build" OFF)

# Function to enable clang-tidy for a target
function(enable_clang_tidy TARGET_NAME)
    find_program(CLANG_TIDY_EXE
        NAMES "clang-tidy-18" "clang-tidy-17" "clang-tidy-16" "clang-tidy"
        DOC "Path to clang-tidy executable"
    )

    if(CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")

        set(CLANG_TIDY_COMMAND
            "${CLANG_TIDY_EXE}"
            "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy"
            "--header-filter=(include|src)/.*"
        )

        set_target_properties(${TARGET_NAME} PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}"
        )

        message(STATUS "clang-tidy enabled for target: ${TARGET_NAME}")
    else()
        message(WARNING "clang-tidy not found. Install with: sudo apt install clang-tidy")
    endif()
endfunction()

# Function to enable cppcheck for a target
function(enable_cppcheck TARGET_NAME)
    find_program(CPPCHECK_EXE
        NAMES "cppcheck"
        DOC "Path to cppcheck executable"
    )

    if(CPPCHECK_EXE)
        message(STATUS "cppcheck found: ${CPPCHECK_EXE}")

        set(CPPCHECK_COMMAND
            "${CPPCHECK_EXE}"
            "--enable=all"
            "--inconclusive"
            "--force"
            "--inline-suppr"
            "--suppressions-list=${CMAKE_SOURCE_DIR}/.cppcheck-suppressions"
            "--std=c++${CMAKE_CXX_STANDARD}"
            "--platform=unix64"
            "--suppress=missingIncludeSystem"
            "--suppress=unmatchedSuppression"
            "--suppress=unusedFunction"
            "-I${CMAKE_SOURCE_DIR}/include"
        )

        set_target_properties(${TARGET_NAME} PROPERTIES
            CXX_CPPCHECK "${CPPCHECK_COMMAND}"
        )

        message(STATUS "cppcheck enabled for target: ${TARGET_NAME}")
    else()
        message(WARNING "cppcheck not found. Install with: sudo apt install cppcheck")
    endif()
endfunction()

# Apply static analysis to target if enabled
function(apply_static_analysis TARGET_NAME)
    if(ENABLE_CLANG_TIDY)
        enable_clang_tidy(${TARGET_NAME})
    endif()

    if(ENABLE_CPPCHECK)
        enable_cppcheck(${TARGET_NAME})
    endif()
endfunction()

# Global clang-tidy configuration
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE
        NAMES "clang-tidy-18" "clang-tidy-17" "clang-tidy-16" "clang-tidy"
    )

    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY
            "${CLANG_TIDY_EXE}"
            "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy"
            "--header-filter=(include|src)/.*"
        )
        message(STATUS "clang-tidy enabled globally")
    endif()
endif()

# Global cppcheck configuration
if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXE NAMES "cppcheck")

    if(CPPCHECK_EXE)
        set(CMAKE_CXX_CPPCHECK
            "${CPPCHECK_EXE}"
            "--enable=all"
            "--inconclusive"
            "--force"
            "--inline-suppr"
            "--suppressions-list=${CMAKE_SOURCE_DIR}/.cppcheck-suppressions"
            "--std=c++${CMAKE_CXX_STANDARD}"
            "--suppress=missingIncludeSystem"
            "--suppress=unmatchedSuppression"
            "--suppress=unusedFunction"
        )
        message(STATUS "cppcheck enabled globally")
    endif()
endif()

# Custom target for running static analysis manually
add_custom_target(static-analysis
    COMMENT "Running static analysis tools..."
)

# Add clang-tidy target if available
find_program(CLANG_TIDY_EXE NAMES "clang-tidy-18" "clang-tidy-17" "clang-tidy" "clang-tidy-16")
if(CLANG_TIDY_EXE)
    add_custom_target(clang-tidy
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/run_clang_tidy.sh ${CMAKE_BINARY_DIR} ${CLANG_TIDY_EXE} ${CMAKE_SOURCE_DIR}/.clang-tidy
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy on source files"
    )
    add_dependencies(static-analysis clang-tidy)
endif()

# Add cppcheck target if available
find_program(CPPCHECK_EXE NAMES "cppcheck")
if(CPPCHECK_EXE)
    add_custom_target(cppcheck
        COMMAND ${CPPCHECK_EXE}
                --enable=all
                --inconclusive
                --force
                --inline-suppr
                --suppressions-list=${CMAKE_SOURCE_DIR}/.cppcheck-suppressions
                --std=c++${CMAKE_CXX_STANDARD}
                --platform=unix64
                --suppress=missingIncludeSystem
                --suppress=unmatchedSuppression
                --suppress=unusedFunction
                -I ${CMAKE_SOURCE_DIR}/include
                ${CMAKE_SOURCE_DIR}/src
                ${CMAKE_SOURCE_DIR}/include
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running cppcheck on source files"
        VERBATIM
    )
    add_dependencies(static-analysis cppcheck)
endif()

