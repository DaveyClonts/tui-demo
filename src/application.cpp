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

  auto view = Renderer([&] { return BuildTui(editor); });

  auto app = CatchEvent(view, [&](Event event) {
    if (event == Event::Escape) {
      screen.ExitLoopClosure()();
      return true;
    }
    if (event == Event::Backspace) {
      editor.Backspace();
      return true;
    }
    if (event == Event::Return) {
      editor.InsertNewline();
      return true;
    }
    if (event.is_character()) {
      editor.Insert(event.character());
      return true;
    }
    return false;
  });

  screen.Loop(app);
  return 0;
}

}  // namespace tui_demo
