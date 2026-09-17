#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "layout.hpp"

int main() {
  using namespace ftxui;

  auto screen = ScreenInteractive::Fullscreen();

  auto layout = Renderer(tui_demo::BuildLayout);

  auto app = CatchEvent(layout, [&](Event event) {
    if (event == Event::Character('q') || event == Event::Character('Q') ||
        event == Event::Escape) {
      screen.ExitLoopClosure()();
      return true;
    }
    return false;
  });

  screen.Loop(app);
  return 0;
}
