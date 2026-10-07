#include "application.hpp"
#include "cli/commands/cli.hpp"

// Keep process startup separate from the terminal application and editing logic.
int main(int argc, char* argv[]) {
  const auto result = tui_demo::ParseCommandLine(argc, argv);
  if (const auto* exit_code = std::get_if<int>(&result)) {
    return *exit_code;
  }

  return tui_demo::RunApplication(std::get<tui_demo::StartupRequest>(result));
}
