#include "application.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <iostream>
#include <memory>
#include <utility>

#include "cli/commands/dispatcher.hpp"
#include "cli/parser.hpp"
#include "tui/editor_pane.hpp"
#include "tui/terminal_pane.hpp"
#include "tui/tui.hpp"

/*
  The Application

  Owns the application state
  Coordiantes events with application_events
  Creates terminal and editor pane and passes them to buildTUI
  Manages liftime
*/
namespace tui_demo {

Application::Application()
    : dispatcher_(editor_), screen_(ftxui::ScreenInteractive::Fullscreen()) {
  editor_pane_ = std::make_shared<EditorPane>(
      editor_, keymap_, [this] { status_message_.clear(); });
  terminal_pane_ = std::make_shared<TerminalPane>(
      status_message_,
      [this](std::string input) { SubmitCommand(std::move(input)); });
}

void Application::SubmitCommand(std::string input) {
  const ParseResult parsed = ParseCommand(std::move(input));
  if (!parsed.command) {
    status_message_ = parsed.message;
    return;
  }
  const CommandResult result = dispatcher_.Dispatch(*parsed.command);
  status_message_ = result.ok() ? "Done" : result.message;
}

int Application::Run(const std::optional<CommandRequest>& request) {
  using namespace ftxui;

  if (request) {
    const CommandResult result = dispatcher_.Dispatch(*request);
    if (!result.ok()) {
      std::cerr << result.message << '\n';
      return 1;
    }
  }

  Component panes = BuildTui(editor_pane_, terminal_pane_);

  // Rebuild the view from current editor state whenever FTXUI draws a frame.
  Component view = Renderer(panes, [&] {
    // FTXUI enables all-motion reporting (1003), which redraws and resets the
    // blinking cursor on every hover event. Request button/drag events (1002).
    // Rendering runs after terminal setup, including setup after Ctrl+Z/resume.
    // Leave tracking enabled in FTXUI so it still restores mouse modes on exit.
    std::cout << "\x1b[?1003l\x1b[?1002h" << std::flush;
    return panes->Render();
  });

  Component app = CatchEvent(
      view, [this](Event event) { return HandleEvent(std::move(event)); });

  // Process input and redraw until a Quit command invokes the exit closure.
  screen_.Loop(app);
  std::cout << "\x1b[?1002l" << std::flush;
  return 0;
}

int RunApplication(const std::optional<CommandRequest>& request) {
  Application application;
  return application.Run(request);
}

} // namespace tui_demo
