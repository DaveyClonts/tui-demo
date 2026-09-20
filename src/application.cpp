#include "application.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "editor/editor.hpp"
#include "tui/tui.hpp"

namespace tui_demo {

int RunApplication() {
  using namespace ftxui;

  auto screen = ScreenInteractive::Fullscreen();
  Editor editor;

  // Rebuild the view from current editor state whenever FTXUI draws a frame.
  auto view = Renderer([&] { return BuildTui(editor); });

  // Translate terminal input into editor operations. true consumes an event;
  // false lets FTXUI pass an unhandled event to the wrapped component.
  auto app = CatchEvent(view, [&](Event event) {
    if (event == Event::Escape) {
      screen.ExitLoopClosure()();
      return true;
    }
    if (event == Event::Backspace) {
      editor.Backspace();
      return true;
    }
    if (event == Event::Delete) {
      editor.Delete();
      return true;
    }
    // FTXUI 7.0.3 has no named Shift+Arrow constants. In these terminal sequences,
    // modifier 2 means Shift and A/B/C/D mean Up/Down/Right/Left.
    if (event == Event::Special("\x1B[1;2D")) {
      editor.MoveLeft(true);
      return true;
    }
    if (event == Event::Special("\x1B[1;2C")) {
      editor.MoveRight(true);
      return true;
    }
    if (event == Event::Special("\x1B[1;2A")) {
      editor.MoveUp(true);
      return true;
    }
    if (event == Event::Special("\x1B[1;2B")) {
      editor.MoveDown(true);
      return true;
    }
    if (event == Event::Return) {
      editor.InsertNewline();
      return true;
    }
    if (event == Event::ArrowLeft) {
      editor.MoveLeft();
      return true;
    }
    if (event == Event::ArrowRight) {
      editor.MoveRight();
      return true;
    }
    if (event == Event::ArrowUp) {
      editor.MoveUp();
      return true;
    }
    if (event == Event::ArrowDown) {
      editor.MoveDown();
      return true;
    }
    // Handle text after special keys so navigation sequences are never inserted.
    if (event.is_character()) {
      editor.Insert(event.character());
      return true;
    }
    return false;
  });

  // Process input and redraw until Escape invokes the exit closure above.
  screen.Loop(app);
  return 0;
}

}  // namespace tui_demo
