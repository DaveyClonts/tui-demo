#include "application.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <iostream>
#include <memory>

#include "cli/commands/dispatcher.hpp"
#include "editor/editor.hpp"
#include "editor/input/keymap.hpp"
#include "tui/editor_pane.hpp"
#include "tui/terminal_pane.hpp"
#include "tui/tui.hpp"

/*
  Application:

  The application

  Should own lifetime details, Editor, PTY, oversee saving and exiting

  Creates Editor and Terminal Panes and passes them into BuildTui
*/
namespace tui_demo {

int RunApplication(const StartupRequest& request) {
  using namespace ftxui;

  Editor editor;
  CommandDispatcher dispatcher(editor);
  if (const auto* open = std::get_if<OpenCommand>(&request)) {
    const auto result = dispatcher.Dispatch(*open);
    if (!result.ok()) {
      std::cerr << "open " << open->path << ": " << result.message << '\n';
      return 1;
    }
  } else if (const auto* create = std::get_if<NewCommand>(&request)) {
    const auto result = dispatcher.Dispatch(*create);
    if (!result.ok()) {
      std::cerr << "new: " << result.message << '\n';
      return 1;
    }
  }

  auto screen = ScreenInteractive::Fullscreen();
  Keymap keymap;
  std::string status_message;
  std::shared_ptr<EditorPane> editor_pane = std::make_shared<EditorPane>(
      editor, keymap, [&] { status_message.clear(); });
  std::shared_ptr<TerminalPane> terminal_pane =
      std::make_shared<TerminalPane>(status_message);
  Component panes = BuildTui(editor_pane, terminal_pane);

  // Rebuild the view from current editor state whenever FTXUI draws a frame.
  Component view = Renderer(panes, [&] {
    // FTXUI enables all-motion reporting (1003), which redraws and resets the
    // blinking cursor on every hover event. Request button/drag events (1002).
    // Rendering runs after terminal setup, including setup after Ctrl+Z/resume.
    // Leave tracking enabled in FTXUI so it still restores mouse modes on exit.
    std::cout << "\x1b[?1003l\x1b[?1002h" << std::flush;
    return panes->Render();
  });

  // Input dispatch reports quit separately so the application owns screen
  // lifetime.
  Component app = CatchEvent(view, [&](Event event) {
    const std::optional<Command> command = keymap.Lookup(event);
    if (command == Command::Save) {
      if (!editor.State().document_open) {
        return true;
      }
      const CommandResult saved = dispatcher.Dispatch(SaveCommand{});
      status_message = saved.ok() ? "Saved" : "Save failed: " + saved.message;
      return true;
    }
    if (command == Command::Quit) {
      screen.ExitLoopClosure()();
      return true;
    }
    if (command == Command::FocusNextPane) {
      if (editor_pane->Focused()) {
        terminal_pane->TakeFocus();
      } else {
        editor_pane->TakeFocus();
      }
      return true;
    }
    if (command == Command::FocusPreviousPane) {
      if (terminal_pane->Focused()) {
        editor_pane->TakeFocus();
      } else {
        terminal_pane->TakeFocus();
      }
      return true;
    }
    return false;
  });

  // Process input and redraw until a Quit command invokes the exit closure.
  screen.Loop(app);
  std::cout << "\x1b[?1002l" << std::flush;
  return 0;
}

} // namespace tui_demo
