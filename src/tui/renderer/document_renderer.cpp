#include "tui/renderer/document_renderer.hpp"

#include <algorithm>
#include <cstddef>
#include <ftxui/dom/elements.hpp>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace tui_demo {
namespace {

struct TextRange {
  std::size_t start;
  std::size_t end;
};

struct DocumentLine {
  std::string_view text;
  TextRange line_range;
  bool has_newline;
};

bool IsUtf8Continuation(char byte) {
  return (static_cast<unsigned char>(byte) & 0xC0) == 0x80;
}

// Positions are byte offsets. Snap an offset inside a character to its start.
std::size_t CharacterStart(std::string_view text, std::size_t position) {
  position = std::min(position, text.size());
  while (position > 0 && position < text.size() &&
         IsUtf8Continuation(text[position])) {
    --position;
  }
  return position;
}

std::size_t NextCharacter(std::string_view text, std::size_t position) {
  if (position >= text.size()) {
    return text.size();
  }
  ++position;
  while (position < text.size() && IsUtf8Continuation(text[position])) {
    ++position;
  }
  return position;
}

TextRange NormalizeSelection(std::size_t cursor_position,
                             std::optional<std::size_t> selection_anchor,
                             std::size_t document_size) {
  const std::size_t cursor = std::min(cursor_position, document_size);
  const std::size_t anchor =
      std::min(selection_anchor.value_or(cursor), document_size);

  return TextRange{
      .start = std::min(anchor, cursor),
      .end = std::max(anchor, cursor),
  };
}

DocumentLine FindLine(std::string_view document,
                      std::size_t line_start_position) {
  const std::size_t newline_position = document.find('\n', line_start_position);
  // npos is returned by .find if char not found
  const bool has_newline = newline_position != std::string_view::npos;

  std::size_t line_end_position;
  if (has_newline) {
    line_end_position = newline_position;
  } else {
    line_end_position = document.size();
  }

  TextRange line_range{
      .start = line_start_position,
      .end = line_end_position,
  };

  const auto document_line_text = document.substr(
      line_start_position, line_end_position - line_start_position);

  return DocumentLine{
      .text = document_line_text,
      .line_range = line_range,
      .has_newline = has_newline,
  };
}

std::vector<std::size_t>
FindStyleBoundaries(std::string_view line,
                    std::optional<std::size_t> cursor_column,
                    TextRange selection) {
  std::vector<std::size_t> boundaries = {0, line.size(), selection.start,
                                         selection.end};
  if (cursor_column) {
    boundaries.push_back(*cursor_column);
    boundaries.push_back(NextCharacter(line, *cursor_column));
  }
  std::sort(boundaries.begin(), boundaries.end());
  boundaries.erase(std::unique(boundaries.begin(), boundaries.end()),
                   boundaries.end());
  return boundaries;
}

ftxui::Element RenderSegment(std::string_view line, TextRange segment,
                             std::optional<std::size_t> cursor_column,
                             TextRange selection, bool show_cursor) {
  ftxui::Element element = ftxui::text(
      std::string{line.substr(segment.start, segment.end - segment.start)});
  if (segment.start >= selection.start && segment.start < selection.end) {
    element = element | ftxui::bgcolor(ftxui::Color::Blue) |
              ftxui::color(ftxui::Color::White);
  }
  if (cursor_column && segment.start == *cursor_column) {
    // Keep the scroll target when unfocused, but hide the editing cursor.
    element = show_cursor ? element | ftxui::focusCursorBarBlinking
                          : element | ftxui::focus;
  }
  return element;
}

ftxui::Element RenderLine(const DocumentLine& line, std::size_t cursor_position,
                          TextRange selection, bool show_cursor) {
  const TextRange range = line.line_range;
  const bool contains_cursor =
      cursor_position >= range.start && cursor_position <= range.end;
  const std::optional<std::size_t> cursor_column =
      contains_cursor
          ? std::optional<std::size_t>{cursor_position - range.start}
          : std::nullopt;
  TextRange local_selection{
      std::clamp(selection.start, range.start, range.end) - range.start,
      std::clamp(selection.end, range.start, range.end) - range.start,
  };
  const bool newline_selected = line.has_newline &&
                                selection.start <= range.end &&
                                selection.end > range.end;

  // A display-only trailing cell shows a selected newline or an end-of-line
  // cursor. This also gives empty lines a cell to highlight without changing
  // the document.
  std::string visible(line.text);
  if (newline_selected ||
      (cursor_column && *cursor_column == line.text.size())) {
    visible += ' ';
  }
  if (newline_selected) {
    local_selection.end = visible.size();
  }

  const std::vector<std::size_t> boundaries =
      FindStyleBoundaries(visible, cursor_column, local_selection);

  ftxui::Elements segments;
  for (std::size_t i = 1; i < boundaries.size(); ++i) {
    const TextRange segment{
        .start = boundaries[i - 1],
        .end = boundaries[i],
    };
    segments.push_back(
        RenderSegment(visible, segment, cursor_column, local_selection, show_cursor));
  }
  return segments.empty() ? ftxui::text("") : ftxui::hbox(std::move(segments));
}

} // namespace

ftxui::Elements RenderDocument(std::string_view document,
                               std::size_t cursor_position,
                               std::optional<std::size_t> selection_anchor,
                               bool show_cursor) {
  cursor_position = CharacterStart(document, cursor_position);
  if (selection_anchor) {
    selection_anchor = CharacterStart(document, *selection_anchor);
  }
  const TextRange selection =
      NormalizeSelection(cursor_position, selection_anchor, document.size());

  ftxui::Elements lines;
  std::size_t line_start = 0;

  // <= preserves an empty document and the blank line after a trailing newline.
  while (line_start <= document.size()) {
    const DocumentLine line = FindLine(document, line_start);
    lines.push_back(RenderLine(line, cursor_position, selection, show_cursor));

    if (!line.has_newline) {
      break;
    }
    line_start = line.line_range.end + 1;
  }

  return lines;
}

} // namespace tui_demo
