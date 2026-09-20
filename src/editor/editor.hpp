#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace tui_demo {

// Document contents and an optional path for future file loading/saving.
struct MarkdownFile {
  std::filesystem::path file;
  std::string text;
};

struct EditorState {
  // Insertion offset in [0, text.size()]. Offsets count bytes, not Unicode characters.
  std::size_t cursor_position = 0;
  // Fixed endpoint while Shift is held; the cursor is the moving endpoint.
  // The selected range is [min(anchor, cursor), max(anchor, cursor)).
  // An anchor equal to the cursor represents an empty selection.
  std::optional<std::size_t> selection_anchor;
  MarkdownFile currentDoc;
};

// Owns editing rules independently of terminal events and presentation.
class Editor {
 public:
  // Insertions replace any selection and leave the cursor after the new text.
  void Insert(std::string_view text);
  void InsertNewline();
  // Remove the selection, or one byte before/at the cursor respectively.
  void Backspace();
  void Delete();
  // selecting=true preserves an anchor so movement extends or shrinks selection.
  // Plain Left/Right collapse a selection; plain Up/Down clear it and move a line.
  void MoveLeft(bool selecting = false);
  void MoveRight(bool selecting = false);
  void MoveUp(bool selecting = false);
  void MoveDown(bool selecting = false);

  // Views may inspect state; mutations go through the editor operations.
  const EditorState& State() const noexcept;

 private:
  // Clears the anchor and returns whether a nonempty range was removed.
  bool DeleteSelection();
  void PrepareSelection(bool selecting);
  void MoveVertically(bool upward);

  EditorState state_;
  // Remember the original column across vertical moves through shorter lines.
  // Horizontal movement and edits reset this preference.
  std::optional<std::size_t> preferred_column_;
};

}  // namespace tui_demo
