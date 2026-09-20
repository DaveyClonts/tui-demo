#include "application.hpp"

#include <iostream>
#include <string_view>

// Keep process startup separate from the terminal application and editing logic.
int main(int argc, char* argv[]) {
  if (argc == 1) {
    return tui_demo::RunApplication();
  }
  // The shell handles quoting, so paths containing spaces arrive as one argument.
  if (argc == 3 && std::string_view(argv[1]) == "open") {
    return tui_demo::RunApplication(std::filesystem::path(argv[2]));
  }
  if (argc == 2 && std::string_view(argv[1]) == "--help") {
    std::cout << "Usage: edit_tui [open <path>]\n";
    return 0;
  }
  std::cerr << "Usage: edit_tui [open <path>]\n";
  return 2;
}
