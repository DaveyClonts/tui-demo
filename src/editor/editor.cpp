#include "editor.hpp"

#include <algorithm>
#include <cctype>
#include <utility>

namespace tui_demo {
namespace {

// Find the first byte of the line containing this insertion position.
std::size_t LineStart(std::string_view document, std::size_t position) {
  if (position == 0) {
    return 0;
  }

  // rfind includes its starting offset. Skip the byte at the cursor so a cursor
  // on a newline still belongs to the line ending there. The guard avoids underflow.
  const auto newline = document.rfind('\n', position - 1);
  return newline == std::string_view::npos ? 0 : newline + 1;
}

}  // namespace

void Editor::LoadDocument(MarkdownFile document) {
  state_.currentDoc = std::move(document);
  state_.document_open = true;
  state_.cursor_position = 0;
  state_.selection_anchor.reset();
  state_.modified = false;
  preferred_column_.reset();
}

void Editor::Insert(std::string_view text) {
  // Deletion moves the cursor to the range start before replacement text is inserted.
  DeleteSelection();
  auto& document_text = state_.currentDoc.text;
  document_text.insert(state_.cursor_position, text);
  state_.modified = state_.modified || !text.empty();
  state_.cursor_position += text.size();
  preferred_column_.reset();
}

void Editor::MarkSaved() noexcept {
  state_.modified = false;
}

void Editor::InsertNewline() {
  Insert("\n");
}

void Editor::Backspace() {
  if (DeleteSelection()) {
    return;
  }
  if (state_.cursor_position == 0) {
    return;
  }

  auto& document_text = state_.currentDoc.text;
  --state_.cursor_position;
  document_text.erase(state_.cursor_position, 1);
  state_.modified = true;
  preferred_column_.reset();
}

bool Editor::DeleteSelection() {
  // Normalize both selection directions; no anchor behaves like an empty range.
  const auto anchor = state_.selection_anchor.value_or(state_.cursor_position);
  const auto start = std::min(anchor, state_.cursor_position);
  const auto end = std::max(anchor, state_.cursor_position);
  state_.selection_anchor.reset();
  if (start == end) {
    return false;
  }
  state_.currentDoc.text.erase(start, end - start);
  state_.modified = true;
  state_.cursor_position = start;
  preferred_column_.reset();
  return true;
}

void Editor::Delete() {
  if (DeleteSelection()) {
    return;
  }
  if (state_.cursor_position < state_.currentDoc.text.size()) {
    state_.currentDoc.text.erase(state_.cursor_position, 1);
    state_.modified = true;
  }
  preferred_column_.reset();
}

void Editor::PrepareSelection(bool selecting) {
  if (selecting) {
    // Capture only once, allowing later moves to cross or return to the anchor.
    if (!state_.selection_anchor) {
      state_.selection_anchor = state_.cursor_position;
    }
  } else {
    state_.selection_anchor.reset();
  }
}

void Editor::MoveLeft(bool selecting) {
  // Collapsing consumes the movement: do not step an extra byte past the range.
  if (!selecting && state_.selection_anchor &&
      *state_.selection_anchor != state_.cursor_position) {
    state_.cursor_position =
        std::min(*state_.selection_anchor, state_.cursor_position);
    PrepareSelection(false);
    preferred_column_.reset();
    return;
  }
  PrepareSelection(selecting);
  if (state_.cursor_position > 0) {
    --state_.cursor_position;
  }
  preferred_column_.reset();
}

void Editor::MoveRight(bool selecting) {
  // The right endpoint is exclusive, so it is already the insertion point after selection.
  if (!selecting && state_.selection_anchor &&
      *state_.selection_anchor != state_.cursor_position) {
    state_.cursor_position =
        std::max(*state_.selection_anchor, state_.cursor_position);
    PrepareSelection(false);
    preferred_column_.reset();
    return;
  }
  PrepareSelection(selecting);
  if (state_.cursor_position < state_.currentDoc.text.size()) {
    ++state_.cursor_position;
  }
  preferred_column_.reset();
}

void Editor::MoveWordLeft(bool selecting) {
  if (!selecting && state_.selection_anchor &&
      *state_.selection_anchor != state_.cursor_position) {
    MoveLeft();
    return;
  }
  PrepareSelection(selecting);
  const auto& text = state_.currentDoc.text;
  auto& position = state_.cursor_position;
  // Skip whitespace, then the preceding word (a run of non-whitespace bytes).
  while (position > 0 &&
         std::isspace(static_cast<unsigned char>(text[position - 1]))) {
    --position;
  }
  while (position > 0 &&
         !std::isspace(static_cast<unsigned char>(text[position - 1]))) {
    --position;
  }
  preferred_column_.reset();
}

void Editor::MoveWordRight(bool selecting) {
  if (!selecting && state_.selection_anchor &&
      *state_.selection_anchor != state_.cursor_position) {
    MoveRight();
    return;
  }
  PrepareSelection(selecting);
  const auto& text = state_.currentDoc.text;
  auto& position = state_.cursor_position;
  // Skip this word and following whitespace to reach the next word's start.
  while (position < text.size() &&
         !std::isspace(static_cast<unsigned char>(text[position]))) {
    ++position;
  }
  while (position < text.size() &&
         std::isspace(static_cast<unsigned char>(text[position]))) {
    ++position;
  }
  preferred_column_.reset();
}

void Editor::MoveUp(bool selecting) {
  PrepareSelection(selecting);
  MoveVertically(true);
}

void Editor::SetCursorPosition(std::size_t position, bool selecting) {
  PrepareSelection(selecting);
  state_.cursor_position = std::min(position, state_.currentDoc.text.size());
  preferred_column_.reset();
}

void Editor::MoveDown(bool selecting) {
  PrepareSelection(selecting);
  MoveVertically(false);
}

void Editor::MoveVertically(bool upward) {
  const auto& document = state_.currentDoc.text;
  const auto current_line_start = LineStart(document, state_.cursor_position);
  std::size_t target_line_start;
  std::size_t target_line_end;

  if (upward) {
    if (current_line_start == 0) {
      return;
    }

    target_line_end = current_line_start - 1;
    target_line_start = LineStart(document, target_line_end);
  } else {
    const auto current_line_end = document.find('\n', state_.cursor_position);
    if (current_line_end == std::string::npos) {
      return;
    }

    // Skip the newline to enter the next line; EOF is the final line's end.
    target_line_start = current_line_end + 1;
    const auto next_newline = document.find('\n', target_line_start);
    target_line_end =
        next_newline == std::string::npos ? document.size() : next_newline;
  }

  // Clamp to the line length while remembering the desired column for later moves.
  const auto target_column =
      preferred_column_.value_or(state_.cursor_position - current_line_start);
  preferred_column_ = target_column;
  state_.cursor_position =
      target_line_start + std::min(target_column, target_line_end - target_line_start);
}

const EditorState& Editor::State() const noexcept {
  return state_;
}

}  // namespace tui_demo
