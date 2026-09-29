cmake_minimum_required(VERSION 3.20)

project(edit_tui LANGUAGES CXX)

# Keep editor language servers in sync with the real CMake build.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Pin the terminal UI dependency so builds use the same API version.
set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  ftxui
  URL https://github.com/ArthurSonzogni/FTXUI/archive/refs/tags/v7.0.3.tar.gz
  URL_HASH SHA256=e7c62ffe19009759821b4f0f8df7f2a6fb83784c3a9f1477d81f56d3ee723c88
  DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)
FetchContent_MakeAvailable(ftxui)

set(CLI11_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(CLI11_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CLI11_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  cli11
  URL https://github.com/CLIUtils/CLI11/archive/refs/tags/v2.4.2.tar.gz
  URL_HASH SHA256=f2d893a65c3b1324c50d4e682c0cdc021dd0477ae2c048544f39eed6654b699a
  DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)
FetchContent_MakeAvailable(cli11)

# Application wiring, editing rules, layout, and text rendering are separate units.
add_executable(edit_tui
  src/application.cpp
  src/commands/cli.cpp
  src/commands/dispatcher.cpp
  src/commands/open.cpp
  src/commands/save.cpp
  src/commands/new.cpp
  src/input/keymap.cpp
  src/editor/editor.cpp
  src/tui/tui.cpp
  src/renderer/renderer.cpp
  src/main.cpp
)
target_compile_features(edit_tui PRIVATE cxx_std_17)
target_include_directories(edit_tui PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(edit_tui PRIVATE ftxui::ftxui CLI11::CLI11)
