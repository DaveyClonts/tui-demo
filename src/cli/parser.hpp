#pragma once

#include <optional>
#include <string>

#include "cli/commands/dispatcher.hpp"

namespace tui_demo {

enum class ParseStatus { Success, Help, Error };

// Successful parsing with no command represents empty input.
struct ParseResult {
  ParseStatus status = ParseStatus::Success;
  std::optional<CommandRequest> command;
  std::string message;
};

ParseResult ParseCommandLine(int argc, char* argv[]);
ParseResult ParseCommand(std::string command_line);

}  // namespace tui_demo
