#include "application.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <iostream>

#include "cli/commands/dispatcher.hpp"
#include "editor/editor.hpp"
#include "editor/input/keymap.hpp"
#include "tui/tui.hpp"

namespace tui_demo {

int RunApplication(const StartupRequest& request) {
  using namespace ftxui;

  Editor editor;
  CommandDispatcher dispatcher(editor);
  // Report startup errors before entering fullscreen so they remain visible in the shell.
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
  InputState input_state;
  std::string status_message;
  DocumentLayout document_layout;

  // Rebuild the view from current editor state whenever FTXUI draws a frame.
  auto view = Renderer([&] {
    // FTXUI enables all-motion reporting (1003), which redraws and resets the
    // blinking cursor on every hover event. Request button/drag events (1002).
    // Rendering runs after terminal setup, including setup after Ctrl+Z/resume.
    // Leave tracking enabled in FTXUI so it still restores mouse modes on exit.
    std::cout << "\x1b[?1003l\x1b[?1002h" << std::flush;
    return BuildTui(editor, status_message, &document_layout);
  });

  // Input dispatch reports quit separately so the application owns screen lifetime.
  auto app = CatchEvent(view, [&](Event event) {
    const auto mouse_position = event.is_mouse()
        ? document_layout.PositionAt(event.mouse().x, event.mouse().y,
                                      editor.State().currentDoc.text)
        : std::nullopt;
    const auto result = HandleInput(event, keymap, editor, input_state, mouse_position);
    if (result == InputResult::Save) {
      const auto saved = dispatcher.Dispatch(SaveCommand{});
      status_message = saved.ok() ? "Saved" : "Save failed: " + saved.message;
    } else if (result == InputResult::Handled) {
      status_message.clear();
    }
    if (result == InputResult::Quit) {
      screen.ExitLoopClosure()();
    }
    return result != InputResult::Unhandled;
  });

  // Process input and redraw until a Quit command invokes the exit closure.
  screen.Loop(app);
  std::cout << "\x1b[?1002l" << std::flush;
  return 0;
}

}  // namespace tui_demo
