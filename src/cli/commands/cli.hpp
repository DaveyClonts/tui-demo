#pragma once

#include <variant>

#include "cli/commands/dispatcher.hpp"

namespace tui_demo {

struct EditUntitled {};

// Each alternative contains exactly the data needed for one startup action.
using StartupRequest = std::variant<EditUntitled, OpenCommand, NewCommand>;

// A request means launch; an exit code means help or a parsing error.
using CliResult = std::variant<StartupRequest, int>;

CliResult ParseCommandLine(int argc, char* argv[]);

}  // namespace tui_demo
