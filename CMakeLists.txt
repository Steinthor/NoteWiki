cmake_minimum_required(VERSION 3.22)

project(notewiki LANGUAGES CXX)

# Global defaults (apply to targets created later)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options so we can later add/remove viewers
option(NOTEWIKI_BUILD_CLI "Build CLI viewer" ON)
option(NOTEWIKI_BUILD_IMGUI "Build ImGui viewer" OFF)

# Header-only library target
add_library(notewiki INTERFACE)
target_include_directories(notewiki INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(notewiki INTERFACE cxx_std_20)

add_library(utilities INTERFACE)
target_include_directories(utilities INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utilities)
target_compile_features(utilities INTERFACE cxx_std_20)

### add the third party json library
add_library(nlohmann_json INTERFACE)
# Use SYSTEM to avoid warning noise from vendored code
target_include_directories(nlohmann_json SYSTEM INTERFACE
  ${CMAKE_SOURCE_DIR}/third_party/nlohmann_json/include
)
# Optional: unify the target name you link against
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
target_link_libraries(notewiki INTERFACE nlohmann_json::nlohmann_json)

# build the CLI app
if (NOTEWIKI_BUILD_CLI)
  add_executable(notewiki_cli apps/cli_viewer/main.cpp)
  target_link_libraries(notewiki_cli PRIVATE utilities)
  target_link_libraries(notewiki_cli PRIVATE notewiki)
endif()

## add tests
find_package(Threads REQUIRED)
# add google test
include(FetchContent)
FetchContent_Declare(
  gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(gtest)

enable_testing()

# We want tests with and without NDEBUG flag
# Common test sources
set(UTILITY_TEST_SOURCES
  tests/utilities/test_color.cpp
  tests/utilities/test_logger.cpp
  tests/utilities/test_logger_fixture.cpp
)

# Debug-flavored tests (no NDEBUG)
add_executable(utility_tests ${UTILITY_TEST_SOURCES})
target_link_libraries(utility_tests PRIVATE utilities GTest::gtest_main)
target_compile_definitions(utility_tests PRIVATE LOGGER_TEST_HOOKS)
add_test(NAME utility_tests COMMAND utility_tests)

# Release-flavored tests (with NDEBUG)
add_executable(utility_tests_release ${UTILITY_TEST_SOURCES})
target_link_libraries(utility_tests_release PRIVATE utilities GTest::gtest_main)
target_compile_definitions(utility_tests_release PRIVATE NDEBUG LOGGER_TEST_HOOKS)
add_test(NAME utility_tests_release COMMAND utility_tests_release)
