#pragma once

#include <optional>
#include <memory>
#include <string>

#include <ftxui/component/screen_interactive.hpp>

#include "cli/commands/dispatcher.hpp"
#include "input/keymap.hpp"
#include "editor/editor.hpp"

namespace tui_demo {

class EditorPane;
class TerminalPane;

class Application {
 public:
  Application();
  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;
  Application(Application&&) = delete;
  Application& operator=(Application&&) = delete;

  int Run(const std::optional<CommandRequest>& request = std::nullopt);

 private:
  bool HandleEvent(ftxui::Event event);
  void SubmitCommand(std::string input);

  // Dependencies precede the components that reference them.
  Editor editor_;
  CommandDispatcher dispatcher_;
  Keymap keymap_;
  std::string status_message_;
  ftxui::ScreenInteractive screen_;
  std::shared_ptr<EditorPane> editor_pane_;
  std::shared_ptr<TerminalPane> terminal_pane_;
};

int RunApplication(const std::optional<CommandRequest>& request = std::nullopt);

} // namespace tui_demo
