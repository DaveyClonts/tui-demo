#include "layout.hpp"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>

#include "editor/editor.hpp"

namespace tui_demo {
namespace {

ftxui::Element RenderLine(std::string_view line,
                          std::optional<std::size_t> cursor_column) {
  if (!cursor_column) {
    return ftxui::text(std::string(line));
  }

  const auto column = std::min(*cursor_column, line.size());
  const auto cursor_width = column < line.size() ? 1U : 0U;
  const auto cursor_character =
      cursor_width == 0 ? std::string(" ")
                        : std::string(line.substr(column, cursor_width));

  return ftxui::hbox({
      ftxui::text(std::string(line.substr(0, column))),
      ftxui::text(cursor_character) | ftxui::focusCursorBarBlinking,
      ftxui::text(std::string(line.substr(column + cursor_width))),
  });
}

ftxui::Elements RenderDocument(std::string_view document,
                               std::size_t cursor_position) {
  ftxui::Elements lines;
  cursor_position = std::min(cursor_position, document.size());
  std::size_t line_start = 0;

  while (line_start <= document.size()) {
    const auto newline = document.find('\n', line_start);
    const auto line_end =
        newline == std::string_view::npos ? document.size() : newline;
    const auto line = document.substr(line_start, line_end - line_start);
    const bool contains_cursor = cursor_position >= line_start &&
                                 cursor_position <= line_end;

    lines.push_back(RenderLine(
        line, contains_cursor
                  ? std::optional(cursor_position - line_start)
                  : std::nullopt));

    if (newline == std::string_view::npos) {
      break;
    }
    line_start = newline + 1;
  }

  return lines;
}

}  // namespace

ftxui::Element BuildLayout(const Editor& editor) {
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
