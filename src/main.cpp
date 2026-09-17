#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

int main() {
  using namespace ftxui;

  auto screen = ScreenInteractive::Fullscreen();

  auto layout = Renderer([&] {
    auto top_pane = vbox({
                        text(" TOP PANE - 75% ") | bold | center,
                        separator(),
                        text("Main content") | center,
                    }) |
                    border | yflex_grow_factor(3);

    auto bottom_pane = vbox({
                           text(" BOTTOM PANE - 25% ") | bold | center,
                           separator(),
                           text("Press Q or Esc to quit") | dim | center,
                       }) |
                       border | yflex_grow_factor(1);

    return vbox({
        top_pane,
        bottom_pane,
    });
  });

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
