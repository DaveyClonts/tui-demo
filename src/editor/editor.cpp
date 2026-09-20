#include "editor.hpp"

#include <algorithm>

namespace tui_demo {
namespace {

std::size_t LineStart(std::string_view document, std::size_t position) {
  if (position == 0) {
    return 0;
  }

  const auto newline = document.rfind('\n', position - 1);
  return newline == std::string_view::npos ? 0 : newline + 1;
}

}  // namespace

void Editor::Insert(std::string_view text) {
  auto& document_text = state_.currentDoc.text;
  document_text.insert(state_.cursor_position, text);
  state_.cursor_position += text.size();
  preferred_column_.reset();
}

void Editor::InsertNewline() {
  Insert("\n");
}

void Editor::Backspace() {
  if (state_.cursor_position == 0) {
    return;
  }

  auto& document_text = state_.currentDoc.text;
  --state_.cursor_position;
  document_text.erase(state_.cursor_position, 1);
  preferred_column_.reset();
}

void Editor::MoveLeft() {
  if (state_.cursor_position > 0) {
    --state_.cursor_position;
  }
  preferred_column_.reset();
}

void Editor::MoveRight() {
  if (state_.cursor_position < state_.currentDoc.text.size()) {
    ++state_.cursor_position;
  }
  preferred_column_.reset();
}

void Editor::MoveUp() {
  MoveVertically(true);
}

void Editor::MoveDown() {
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

    const auto previous_line_end = current_line_start - 1;
    const auto previous_line_start = LineStart(document, previous_line_end);
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
