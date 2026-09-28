#include "commands/cli.hpp"

#include <CLI/CLI.hpp>
#include <string>

namespace tui_demo {

CliResult ParseCommandLine(int argc, char* argv[]) {
  CLI::App app{"A small terminal editor"};
  app.require_subcommand(0, 1);

  std::string path;
  auto* open = app.add_subcommand("open", "Open a file");
  open->add_option("path", path, "File to open")->required();

  auto* new_command = app.add_subcommand(
      "new", "Create and open a new Markdown file in the current directory");

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError& error) {
    const int code = app.exit(error);
    // Preserve the application's exit codes for help and invalid arguments.
    return code == 0 ? 0 : 2;
  }

  StartupOptions options;
  if (*open) {
    options.action = StartupAction::Open;
    options.initial_path = std::filesystem::path{path};
  } else if (*new_command) {
    options.action = StartupAction::New;
  }
  return options;
}

}  // namespace tui_demo
