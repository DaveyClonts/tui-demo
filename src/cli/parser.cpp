#include "cli/parser.hpp"

#include <CLI/CLI.hpp>
#include <string>
#include <utility>

namespace tui_demo {
namespace {

// Both entry points share command definitions, but save requires a running
// editor.
struct CommandParser {
  std::string path;
  CLI::App app{"A small terminal editor"};
  CLI::App* open;
  CLI::App* new_command;
  CLI::App* save = nullptr;

  explicit CommandParser(bool interactive) {
    app.require_subcommand(0, 1);
    open = app.add_subcommand("open", "Open a file");
    open->add_option("path", path, "File to open")->required();
    new_command = app.add_subcommand(
        "new", "Create and open a new Markdown file in the current directory");
    if (interactive) {
      save = app.add_subcommand("save", "Save the current file");
    }
  }

  // CLI11 accepts both argc/argv and a command string. Keep result conversion
  // and diagnostics identical regardless of the input representation.
  template <typename... Arguments> ParseResult Parse(Arguments&&... arguments) {
    try {
      app.parse(std::forward<Arguments>(arguments)...);
    } catch (const CLI::CallForHelp&) {
      return {ParseStatus::Help, std::nullopt, app.help()};
    } catch (const CLI::ParseError& error) {
      return {ParseStatus::Error, std::nullopt, error.what()};
    }

    if (*open) {
      return {
          ParseStatus::Success, OpenCommand{std::filesystem::path{path}}, {}};
    }
    if (*new_command) {
      return {ParseStatus::Success, NewCommand{}, {}};
    }
    if (save && *save) {
      return {ParseStatus::Success, SaveCommand{}, {}};
    }
    return {};
  }
};

} // namespace

ParseResult ParseCommandLine(int argc, char* argv[]) {
  CommandParser parser(false);
  return parser.Parse(argc, argv);
}

ParseResult ParseCommand(std::string command_line) {
  CommandParser parser(true);
  return parser.Parse(std::move(command_line));
}

} // namespace tui_demo
