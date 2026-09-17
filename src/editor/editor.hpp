#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace tui_demo {

enum class NewLineStyle {
  NewLine,
  CarriageReturnNewLine,
};

struct EditorState {
  // Positions are UTF-8 byte offsets into MarkdownDocument::text. Editing
  // operations must keep them on character boundaries.
  std::size_t cursor = 0;
  std::size_t selection_anchor = 0;

  std::size_t scroll_row = 0;
  std::size_t scroll_column = 0;
  std::size_t preferred_column = 0;
};

struct MarkdownDocument {
  std::filesystem::path path;
  std::string text;
  NewLineStyle new_line_style = NewLineStyle::NewLine;
  bool modified = false;
};

class Editor {
 public:
  Editor() = default;

  void NewDocument();
  bool Open(const std::filesystem::path& path);
  bool Save();
  bool SaveAs(const std::filesystem::path& path);

  void Insert(std::string_view text);
  void InsertNewline();
  void Backspace();
  void DeleteForward();

  // When extend_selection is true, the selection anchor remains fixed while
  // the cursor moves. This is how Shift+Arrow selection is represented.
  void MoveLeft(bool extend_selection = false);
  void MoveRight(bool extend_selection = false);
  void MoveUp(bool extend_selection = false);
  void MoveDown(bool extend_selection = false);
  void MoveToLineStart(bool extend_selection = false);
  void MoveToLineEnd(bool extend_selection = false);

  void SelectAll();
  void ClearSelection();
  bool HasSelection() const noexcept;

  bool CanUndo() const noexcept;
  bool CanRedo() const noexcept;
  void Undo();
  void Redo();

  const MarkdownDocument& Document() const noexcept;
  const EditorState& State() const noexcept;
  const std::string& LastError() const noexcept;

 private:
  struct Snapshot {
    std::string text;
    std::size_t cursor = 0;
    std::size_t selection_anchor = 0;
  };

  void PushUndoState();
  void RestoreSnapshot(Snapshot snapshot);
  void DeleteSelection();
  void MoveCursorTo(std::size_t position, bool extend_selection);
  void ClampPositions();
  void UpdateModified();

  MarkdownDocument document_;
  EditorState state_;
  std::string saved_text_;
  std::string last_error_;
  std::vector<Snapshot> undo_stack_;
  std::vector<Snapshot> redo_stack_;
};

}  // namespace tui_demo
