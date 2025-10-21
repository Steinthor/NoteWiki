cmake_minimum_required(VERSION 3.22)

project(notewiki LANGUAGES CXX)

# Global defaults (apply to targets created later)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options so we can later add/remove viewers
option(NOTEWIKI_BUILD_CLI "Build CLI viewer" ON)
option(NOTEWIKI_BUILD_IMGUI "Build ImGui viewer" ON)

# Header-only library target
add_library(notewiki INTERFACE)
target_include_directories(notewiki INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(notewiki INTERFACE cxx_std_20)

add_library(utilities INTERFACE)
target_include_directories(utilities INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utilities)
target_compile_features(utilities INTERFACE cxx_std_20)

#
# add the third party library cxxopts
#
add_subdirectory(third_party/cxxopts)

#
# add the third party json library
#
add_library(nlohmann_json INTERFACE)
# Use SYSTEM to avoid warning noise from vendored code
target_include_directories(nlohmann_json SYSTEM INTERFACE
  ${CMAKE_SOURCE_DIR}/third_party/nlohmann_json/include
)
# Optional: unify the target name you link against
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
target_link_libraries(notewiki INTERFACE nlohmann_json::nlohmann_json)

#
# add the third party glfw library
#
include(FetchContent)

find_package(OpenGL REQUIRED)

# GLFW
find_package(glfw3 QUIET)
if (NOT glfw3_FOUND)
  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
  )
  FetchContent_MakeAvailable(glfw)          # defines target 'glfw'
  add_library(glfw::glfw ALIAS glfw)        # stable alias
endif()


#
# add the third party ImgUI library
#

add_library(imgui STATIC
  ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui.cpp
  ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_draw.cpp
  ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_tables.cpp
  ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_widgets.cpp
  ${CMAKE_SOURCE_DIR}/third_party/imgui/misc/cpp/imgui_stdlib.cpp
  ${CMAKE_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_glfw.cpp
  ${CMAKE_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_opengl3.cpp
  ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_demo.cpp
)

# Choose backends: GLFW + OpenGL3
target_sources(imgui PRIVATE
  ${CMAKE_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_glfw.cpp
  ${CMAKE_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_opengl3.cpp
)

# Headers for core and backends
target_include_directories(imgui 
  PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/imgui
    ${CMAKE_SOURCE_DIR}/third_party/imgui/backends
  PRIVATE
    $<TARGET_PROPERTY:glfw::glfw,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(imgui PUBLIC glfw::glfw OpenGL::GL)

#
# build the CLI app
#
if (NOTEWIKI_BUILD_CLI)
  add_executable(notewiki_cli apps/cli_viewer/main.cpp)
  target_link_libraries(notewiki_cli PRIVATE notewiki utilities cxxopts)
endif()

#
# build the ImgUI app
#
if (NOTEWIKI_BUILD_IMGUI)
  # copy fonts to build folder
  set(FONTS_SRC_DIR "${CMAKE_SOURCE_DIR}/third_party/fonts")
  set(FONTS_DST_DIR "${CMAKE_BINARY_DIR}/fonts")
  # Collect *.ttf and *.otf (re-configure when files are added/removed)
  file(GLOB FONT_FILES CONFIGURE_DEPENDS
      "${FONTS_SRC_DIR}/*.ttf"
      "${FONTS_SRC_DIR}/*.otf"
  )
  add_custom_target(copy_fonts ALL
      COMMAND ${CMAKE_COMMAND} -E make_directory "${FONTS_DST_DIR}"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FONT_FILES} "${FONTS_DST_DIR}"
      DEPENDS ${FONT_FILES}
      COMMENT "Copying fonts to build/fonts"
  )

  add_executable(notewiki_ui apps/imgui_viewer/main.cpp)
  target_link_libraries(notewiki_ui PRIVATE notewiki utilities imgui glfw::glfw cxxopts)
  add_dependencies(notewiki_ui copy_fonts)
endif()

#
# add tests
#
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
