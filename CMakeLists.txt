cmake_minimum_required(VERSION 3.20)

project(ftxui_split_demo LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG v7.0.3
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(ftxui)

add_executable(ftxui_split_demo src/main.cpp)
target_compile_features(ftxui_split_demo PRIVATE cxx_std_17)
target_link_libraries(ftxui_split_demo PRIVATE ftxui::ftxui)

