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

  if (*open) {
    return StartupRequest{OpenCommand{std::filesystem::path{path}}};
  }
  if (*new_command) {
    return StartupRequest{NewCommand{}};
  }
  return StartupRequest{EditUntitled{}};
}

}  // namespace tui_demo
