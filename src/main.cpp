#include "application.hpp"
#include "cli/parser.hpp"

#include <iostream>

// Keep process startup separate from the terminal application and editing logic.
int main(int argc, char* argv[]) {
  const tui_demo::ParseResult result = tui_demo::ParseCommandLine(argc, argv);
  if (result.status == tui_demo::ParseStatus::Help) {
    std::cout << result.message;
    return 0;
  }
  if (result.status == tui_demo::ParseStatus::Error) {
    std::cerr << result.message << '\n';
    return 2;
  }

  return tui_demo::RunApplication(result.command);
}
