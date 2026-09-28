#pragma once

#include <filesystem>
#include <optional>
#include <variant>

namespace tui_demo {

enum class StartupAction {
  EditUntitled,
  Open,
  New,
};

struct StartupOptions {
  StartupAction action = StartupAction::EditUntitled;
  std::optional<std::filesystem::path> initial_path;
};

// Options mean launch; an exit code means help or a parsing error.
using CliResult = std::variant<StartupOptions, int>;

CliResult ParseCommandLine(int argc, char* argv[]);

}  // namespace tui_demo
