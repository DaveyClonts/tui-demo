#include "tui/tui.hpp"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>

#include <algorithm>

#include "editor/editor.hpp"
#include "renderer/renderer.hpp"

namespace tui_demo {

ftxui::Element BuildTui(const Editor& editor, const std::string& status_message) {
  using namespace ftxui;

  // Recompute proportions on each render so the layout follows terminal resizing.
  // Minimum dimensions reserve room for borders and the help text.
  const int left_pane_width = std::max(3, Terminal::Size().dimx * 15 / 100);
  const int bottom_pane_height =
      std::max(6, Terminal::Size().dimy * 20 / 100);

  // Reserved sidebar; the document occupies the flexible area to its right.
  auto left_pane = emptyElement() | border |
                   size(WIDTH, EQUAL, left_pane_width) | yflex;

  const auto& editor_state = editor.State();
  // frame keeps the focused cursor visible; flex lets the viewport fit the pane.
  auto top_pane =
      vbox(RenderDocument(editor_state.currentDoc.text,
                          editor_state.cursor_position,
                          editor_state.selection_anchor)) |
      vscroll_indicator | frame | flex | border | yflex;

  // This is a static help pane, not an embedded shell or terminal process.
  auto bottom_pane = vbox({
                         text(" Terminal ") | bold | center,
                         separator(),
                         text("Ctrl+S: save | Esc: quit") | dim | center,
                         text(status_message) | center,
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
