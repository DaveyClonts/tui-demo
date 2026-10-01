#include "renderer/renderer.hpp"

#include <algorithm>
#include <ftxui/dom/elements.hpp>
#include <optional>
#include <string>
#include <vector>
#include <utility>

namespace tui_demo {
namespace {

std::vector<std::size_t> FindStyleBoundaries(
    std::size_t line_size,
    std::optional<std::size_t> cursor_column,
    std::size_t selection_start,
    std::size_t selection_end) {
  std::vector<std::size_t> boundaries = {
      0, line_size, selection_start, selection_end};
  if (cursor_column) {
    boundaries.push_back(*cursor_column);
    boundaries.push_back(*cursor_column + 1);
  }
  std::sort(boundaries.begin(), boundaries.end());
  boundaries.erase(std::unique(boundaries.begin(), boundaries.end()),
                   boundaries.end());
  return boundaries;
}

ftxui::Element RenderPiece(std::string_view line,
                           std::size_t start,
                           std::size_t end,
                           std::optional<std::size_t> cursor_column,
                           std::size_t selection_start,
                           std::size_t selection_end) {
  auto piece = ftxui::text(std::string{line.substr(start, end - start)});
  if (start >= selection_start && start < selection_end) {
    piece = piece | ftxui::bgcolor(ftxui::Color::Blue) |
            ftxui::color(ftxui::Color::White);
  }
  if (cursor_column && start == *cursor_column) {
    piece = piece | ftxui::focusCursorBarBlinking;
  }
  return piece;
}

// Inputs are offsets relative to this line; line excludes its newline byte.
ftxui::Element RenderLine(std::string_view line,
                          std::optional<std::size_t> cursor_column,
                          std::size_t selection_start,
                          std::size_t selection_end,
                          bool newline_selected) {
  // A display-only trailing cell shows a selected newline or an end-of-line cursor.
  // This also gives empty lines a cell to highlight without changing the document.
  std::string visible(line);
  if (newline_selected || (cursor_column && *cursor_column == line.size())) {
    visible += ' ';
  }
  if (newline_selected) {
    selection_end = visible.size();
  }

  const auto boundaries = FindStyleBoundaries(
      visible.size(), cursor_column, selection_start, selection_end);

  ftxui::Elements pieces;
  for (std::size_t i = 1; i < boundaries.size(); ++i) {
    pieces.push_back(RenderPiece(
        visible, boundaries[i - 1], boundaries[i], cursor_column,
        selection_start, selection_end));
  }
  return pieces.empty() ? ftxui::text("") : ftxui::hbox(std::move(pieces));
}

}  // namespace


ftxui::Elements RenderDocument(std::string_view document,
                               std::size_t cursor_position,
                               std::optional<std::size_t> selection_anchor) {
  ftxui::Elements lines;

  // Dont let cursor_position exceed document size
  cursor_position = std::min(cursor_position, document.size());

  const auto anchor = std::min(selection_anchor.value_or(cursor_position),
                               document.size());
  const auto selection_start = std::min(anchor, cursor_position);
  const auto selection_end = std::max(anchor, cursor_position);
  std::size_t line_start = 0;

  // <= preserves an empty document and the blank line after a trailing newline.
  while (line_start <= document.size()) {
    const auto newline = document.find('\n', line_start);
    const auto line_end =
        newline == std::string_view::npos ? document.size() : newline;
    const auto line = document.substr(line_start, line_end - line_start);
    // Text excludes line_end, but the cursor may sit there just before the newline.
    const bool contains_cursor = cursor_position >= line_start &&
                                 cursor_position <= line_end;

    // Clip selection to this line and convert document offsets to local columns.
    // Newline selection is separate because the newline is absent from the line text.
    lines.push_back(RenderLine(
        line, contains_cursor
                  ? std::optional(cursor_position - line_start)
                  : std::nullopt,
        std::clamp(selection_start, line_start, line_end) - line_start,
        std::clamp(selection_end, line_start, line_end) - line_start,
        newline != std::string_view::npos && selection_start <= line_end &&
            selection_end > line_end));

    if (newline == std::string_view::npos) {
      break;
    }
    line_start = newline + 1;
  }

  return lines;
}

ftxui::Elements RenderDocument2(std::string_view document, 
  std::size_t cursor_position,
  std::optional<std::size_t> selection_anchor) {
    
    ftxui::Elements lines;

    // Doesn't let cursor_position exceed document size
    cursor_position = std::min(cursor_position, document.size());

    //TODO: figure out selection logic

    std::size_t line_start_position = 0;
    while (line_start_position <= document.size()) {
      const size_t newline_position = document.find('\n', line_start_position);

      size_t line_end_position;
      const size_t max_size_t_value = std::string_view::npos;

      if (newline_position == max_size_t_value) {
        line_end_position = document.size();
      } else {
        line_end_position = newline_position;
      }
      
      const size_t line_size = line_end_position - line_start_position; 
      const auto line = document.substr(line_start_position, line_size);
      
      const bool line_contains_cursor = cursor_position >= line_start_position && 
                                        cursor_position <= line_end_position;

      //TODO: figure out renderLine mess
    }
}

}  // namespace tui_demo
