#include "layout.hpp"

#include <ftxui/dom/elements.hpp>

namespace tui_demo {

ftxui::Element BuildLayout() {
  using namespace ftxui;

  auto left_pane = emptyElement() | borderLight | xflex_grow_factor(15) |
                   yflex;

  auto top_pane = emptyElement() | borderLight | yflex_grow_factor(5);

  auto bottom_pane = vbox({
                         text(" Terminal ") | bold | center,
                         separator(),
                         text("Press Q or Esc to quit") | dim | center,
                     }) |
                     borderLight | yflex_grow_factor(1);

  auto right_pane = vbox({
                        top_pane,
                        bottom_pane,
                    }) |
                    xflex_grow_factor(85) | yflex;

  return hbox({
      left_pane,
      right_pane,
  });
}

}  // namespace tui_demo

