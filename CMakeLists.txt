cmake_minimum_required(VERSION 3.20)

project(edit_tui LANGUAGES CXX)

# Keep editor language servers in sync with the real CMake build.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Pin the terminal UI dependency so builds use the same API version.
FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG v7.0.3
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(ftxui)

# Application wiring, editing rules, layout, and text rendering are separate units.
add_executable(edit_tui
  src/application.cpp
  src/editor/editor.cpp
  src/tui/tui.cpp
  src/renderer/renderer.cpp
  src/main.cpp
)
target_compile_features(edit_tui PRIVATE cxx_std_17)
target_include_directories(edit_tui PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(edit_tui PRIVATE ftxui::ftxui)

# Exercise editor behavior and render to an in-memory screen without a terminal loop.
# BUILD_TESTING can disable this target for application-only builds.
include(CTest)
if(BUILD_TESTING)
  add_executable(selection_tests
    tests/selection_tests.cpp
    src/editor/editor.cpp
    src/renderer/renderer.cpp
  )
  target_compile_features(selection_tests PRIVATE cxx_std_17)
  target_include_directories(selection_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
  target_link_libraries(selection_tests PRIVATE ftxui::ftxui)
  add_test(NAME selection_tests COMMAND selection_tests)
endif()
