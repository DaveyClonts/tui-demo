#include "editor.hpp"

namespace tui_demo {

void Editor::Insert(std::string_view text) {
  auto& document_text = state_.currentDoc.text;
  document_text.insert(state_.cursor_position, text);
  state_.cursor_position += text.size();
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
}

const EditorState& Editor::State() const noexcept {
  return state_;
}

}  // namespace tui_demo
