#include "editor.hpp"

#include <algorithm>
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

void Editor::MoveUp(bool selecting) {
  PrepareSelection(selecting);
  MoveVertically(true);
}

void Editor::MoveDown(bool selecting) {
  PrepareSelection(selecting);
  MoveVertically(false);
}

void Editor::MoveVertically(bool upward) {
  const auto& document = state_.currentDoc.text;
  const auto current_line_start = LineStart(document, state_.cursor_position);
  const auto current_column = state_.cursor_position - current_line_start;

  if (upward) {
    if (current_line_start == 0) {
      return;
    }

    // A line starts immediately after a newline
    const auto previous_line_end = current_line_start - 1;
    const auto previous_line_start = LineStart(document, previous_line_end);

    // Clamp this move to the line length but retain the desired column for later moves.
    const auto target_column = preferred_column_.value_or(current_column);
    preferred_column_ = target_column;
    state_.cursor_position =
        previous_line_start +
        std::min(target_column, previous_line_end - previous_line_start);
    return;
  }

  const auto current_line_end = document.find('\n', state_.cursor_position);
  if (current_line_end == std::string::npos) {
    return;
  }

  // Skip the newline to enter the next line; EOF is the final line's end.
  const auto next_line_start = current_line_end + 1;
  const auto next_newline = document.find('\n', next_line_start);
  const auto next_line_end =
      next_newline == std::string::npos ? document.size() : next_newline;
  const auto target_column = preferred_column_.value_or(current_column);
  preferred_column_ = target_column;
  state_.cursor_position =
      next_line_start + std::min(target_column, next_line_end - next_line_start);
}

const EditorState& Editor::State() const noexcept {
  return state_;
}

}  // namespace tui_demo
