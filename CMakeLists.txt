cmake_minimum_required(VERSION 3.16)
project(git2prompt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)
option(USE_SIMDJSON "Use simdjson for JSON output" ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  if(NOT WIN32)  # or use something like if(UNIX)
    add_compile_options(-flto)
  endif()
  add_compile_options(-O3 -march=native)
endif()


include(FetchContent)

# ----------------------------------------------------------------------------
# CLI11
# ----------------------------------------------------------------------------
FetchContent_Declare(
  CLI11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG        v2.3.2
)
FetchContent_MakeAvailable(CLI11)

# ----------------------------------------------------------------------------
# spdlog
# ----------------------------------------------------------------------------
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.11.0
)
FetchContent_MakeAvailable(spdlog)

# ----------------------------------------------------------------------------
# simdjson (optional)
# ----------------------------------------------------------------------------
if(USE_SIMDJSON)
  FetchContent_Declare(
    simdjson
    GIT_REPOSITORY https://github.com/simdjson/simdjson.git
    GIT_TAG        v3.0.0
  )
  FetchContent_MakeAvailable(simdjson)
endif()

# ----------------------------------------------------------------------------
# GoogleTest (for tests, if enabled)
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
  include(CTest)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.12.1
  )
  FetchContent_MakeAvailable(googletest)
endif()

# ----------------------------------------------------------------------------
# Build library (common source files)
# ----------------------------------------------------------------------------
set(LIB_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/comment_scrub.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/output_formatter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/processor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/repo_scanner.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/token_count.cpp"
)

add_library(git2prompt-lib STATIC ${LIB_SOURCES})
target_include_directories(git2prompt-lib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(git2prompt-lib
    PUBLIC
        CLI11::CLI11
        spdlog::spdlog
)

if(USE_SIMDJSON)
    target_compile_definitions(git2prompt-lib PRIVATE -DUSE_SIMDJSON)
    target_link_libraries(git2prompt-lib PRIVATE simdjson)
endif()

# ----------------------------------------------------------------------------
# Build the CLI executable
# ----------------------------------------------------------------------------
add_executable(git2prompt "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
target_link_libraries(git2prompt PRIVATE git2prompt-lib)

# ----------------------------------------------------------------------------
# Tests
# ----------------------------------------------------------------------------
if(BUILD_TESTS)
    file(GLOB TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
    add_executable(git2prompt-tests ${TEST_FILES})
    target_include_directories(git2prompt-tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_link_libraries(git2prompt-tests
        PRIVATE
            git2prompt-lib
            gtest_main
    )
    include(GoogleTest)
    gtest_discover_tests(git2prompt-tests)
endif()
