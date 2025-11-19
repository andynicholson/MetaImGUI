# Code Coverage Configuration
# Adds code coverage support for GCC, Clang, and MSVC

option(ENABLE_COVERAGE "Enable code coverage" OFF)

# Function to setup coverage compiler flags
function(setup_coverage_flags)
    if(ENABLE_COVERAGE)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            message(STATUS "Code coverage enabled for ${CMAKE_CXX_COMPILER_ID}")

            # GCC and Clang coverage flags
            set(COVERAGE_COMPILER_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage" PARENT_SCOPE)
            set(COVERAGE_LINKER_FLAGS "--coverage" PARENT_SCOPE)

            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage -fprofile-arcs -ftest-coverage" PARENT_SCOPE)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage -fprofile-arcs -ftest-coverage" PARENT_SCOPE)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage" PARENT_SCOPE)
            set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage" PARENT_SCOPE)

        elseif(MSVC)
            message(STATUS "For MSVC, use OpenCppCoverage: https://github.com/OpenCppCoverage/OpenCppCoverage")
            message(STATUS "Example: OpenCppCoverage.exe --sources MetaImGUI -- MetaImGUI_tests.exe")
        else()
            message(WARNING "Code coverage is not supported for compiler: ${CMAKE_CXX_COMPILER_ID}")
        endif()
    endif()
endfunction()

# Function to add coverage target
function(add_coverage_target TARGET_NAME)
    if(ENABLE_COVERAGE AND (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"))
        # Find required tools
        find_program(LCOV_PATH lcov)
        find_program(GENHTML_PATH genhtml)

        if(LCOV_PATH AND GENHTML_PATH)
            # Add custom target for coverage report generation
            add_custom_target(coverage
                # Cleanup lcov
                COMMAND ${LCOV_PATH} --directory . --zerocounters

                # Run tests
                COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure

                # Capture coverage data
                COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info

                # Remove external and test files from coverage
                COMMAND ${LCOV_PATH} --remove coverage.info
                    '/usr/*'
                    '${CMAKE_SOURCE_DIR}/external/*'
                    '${CMAKE_SOURCE_DIR}/tests/*'
                    --output-file coverage.info.cleaned

                # Generate HTML report
                COMMAND ${GENHTML_PATH} -o coverage_report coverage.info.cleaned

                # Print summary
                COMMAND ${LCOV_PATH} --list coverage.info.cleaned

                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating code coverage report..."
                VERBATIM
            )

            message(STATUS "Coverage target 'coverage' created. Run: make coverage")
        else()
            message(WARNING "lcov and/or genhtml not found. Coverage target not created.")
            message(STATUS "Install with: sudo apt-get install lcov (Ubuntu) or brew install lcov (macOS)")
        endif()
    endif()
endfunction()

