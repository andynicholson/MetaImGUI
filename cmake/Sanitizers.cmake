# Sanitizers Configuration
# Adds support for various sanitizers (ASAN, TSAN, UBSAN, MSAN)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_MSAN "Enable MemorySanitizer" OFF)

# Check if any sanitizer is enabled
if(ENABLE_ASAN OR ENABLE_TSAN OR ENABLE_UBSAN OR ENABLE_MSAN)
    if(NOT (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"))
        message(WARNING "Sanitizers are only supported with GCC and Clang")
        return()
    endif()

    # Sanitizers require debug symbols
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        message(WARNING "Sanitizers work best with Debug or RelWithDebInfo build type")
    endif()
endif()

# AddressSanitizer (ASAN) - Detects memory errors
if(ENABLE_ASAN)
    message(STATUS "AddressSanitizer enabled")

    # ASAN flags
    set(ASAN_FLAGS "-fsanitize=address -fno-omit-frame-pointer -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ASAN_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ASAN_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")

    # ASAN options for better error reporting
    set(ENV{ASAN_OPTIONS} "detect_leaks=1:check_initialization_order=1:strict_init_order=1")
endif()

# ThreadSanitizer (TSAN) - Detects data races
if(ENABLE_TSAN)
    message(STATUS "ThreadSanitizer enabled")

    if(ENABLE_ASAN)
        message(FATAL_ERROR "ThreadSanitizer cannot be used together with AddressSanitizer")
    endif()

    # TSAN flags
    set(TSAN_FLAGS "-fsanitize=thread -fno-omit-frame-pointer -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TSAN_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TSAN_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=thread")

    # TSAN options
    set(ENV{TSAN_OPTIONS} "second_deadlock_stack=1")
endif()

# UndefinedBehaviorSanitizer (UBSAN) - Detects undefined behavior
if(ENABLE_UBSAN)
    message(STATUS "UndefinedBehaviorSanitizer enabled")

    # UBSAN flags
    set(UBSAN_FLAGS "-fsanitize=undefined -fno-omit-frame-pointer -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${UBSAN_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${UBSAN_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=undefined")

    # UBSAN options
    set(ENV{UBSAN_OPTIONS} "print_stacktrace=1")
endif()

# MemorySanitizer (MSAN) - Detects uninitialized memory reads
if(ENABLE_MSAN)
    message(STATUS "MemorySanitizer enabled")

    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "MemorySanitizer is only supported with Clang")
    endif()

    if(ENABLE_ASAN OR ENABLE_TSAN)
        message(FATAL_ERROR "MemorySanitizer cannot be used with other sanitizers")
    endif()

    # MSAN flags
    set(MSAN_FLAGS "-fsanitize=memory -fno-omit-frame-pointer -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSAN_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MSAN_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=memory")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=memory")
endif()

# Helper function to check if sanitizers are enabled
function(sanitizers_enabled OUTPUT_VAR)
    if(ENABLE_ASAN OR ENABLE_TSAN OR ENABLE_UBSAN OR ENABLE_MSAN)
        set(${OUTPUT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${OUTPUT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

# Print summary
if(ENABLE_ASAN OR ENABLE_TSAN OR ENABLE_UBSAN OR ENABLE_MSAN)
    message(STATUS "Sanitizers summary:")
    if(ENABLE_ASAN)
        message(STATUS "  - AddressSanitizer: ON")
    endif()
    if(ENABLE_TSAN)
        message(STATUS "  - ThreadSanitizer: ON")
    endif()
    if(ENABLE_UBSAN)
        message(STATUS "  - UndefinedBehaviorSanitizer: ON")
    endif()
    if(ENABLE_MSAN)
        message(STATUS "  - MemorySanitizer: ON")
    endif()
    message(STATUS "Note: Sanitizers may significantly slow down execution")
endif()

