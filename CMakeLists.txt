cmake_minimum_required(VERSION 3.22)
project(notewiki LANGUAGES CXX)

# Options so we can later add/remove viewers
option(NOTEWIKI_BUILD_CLI "Build CLI viewer" ON)
option(NOTEWIKI_BUILD_IMGUI "Build ImGui viewer" OFF)

# Header-only library target
add_library(notewiki INTERFACE)
target_include_directories(notewiki INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(notewiki INTERFACE cxx_std_20)

### add the third party json library
add_library(nlohmann_json INTERFACE)
# Use SYSTEM to avoid warning noise from vendored code
target_include_directories(nlohmann_json SYSTEM INTERFACE
  ${CMAKE_SOURCE_DIR}/third_party/nlohmann_json/include
)
# Optional: unify the target name you link against
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
target_link_libraries(notewiki INTERFACE nlohmann_json::nlohmann_json)

if (NOTEWIKI_BUILD_CLI)
  add_executable(notewiki_cli apps/cli_viewer/main.cpp)
  target_link_libraries(notewiki_cli PRIVATE notewiki)
endif()