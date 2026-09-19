#include "tui/tui.hpp"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>

#include <algorithm>

#include "editor/editor.hpp"
#include "tui/renderer/renderer.hpp"

namespace tui_demo {

ftxui::Element BuildTui(const Editor& editor) {
  using namespace ftxui;

  const int left_pane_width = std::max(3, Terminal::Size().dimx * 15 / 100);
  const int bottom_pane_height =
      std::max(5, Terminal::Size().dimy * 20 / 100);

  auto left_pane = emptyElement() | border |
                   size(WIDTH, EQUAL, left_pane_width) | yflex;

  const auto& editor_state = editor.State();
  auto top_pane =
      vbox(RenderDocument(editor_state.currentDoc.text,
                          editor_state.cursor_position)) |
      vscroll_indicator | frame | flex | border | yflex;

  auto bottom_pane = vbox({
                         text(" Terminal ") | bold | center,
                         separator(),
                         text("Press Esc to quit") | dim | center,
                     }) |
                     border | size(HEIGHT, EQUAL, bottom_pane_height);

  auto right_pane = vbox({
                        top_pane,
                        bottom_pane,
                    }) |
                    xflex | yflex;

  return hbox({
      left_pane,
      right_pane,
  });
}

}  // namespace tui_demo
