#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace tui_demo {

struct MarkdownFile {
  std::filesystem::path file;
  std::string text;
};

struct EditorState {
  std::size_t cursor_position = 0;
  std::optional<std::size_t> selection_anchor;
  MarkdownFile currentDoc;
  bool document_open = false;
  bool modified = false;
};

class Editor {
 public:
  void LoadDocument(MarkdownFile document);
  void MarkSaved() noexcept;
  void Insert(std::string_view text);
  void InsertNewline();
  void Backspace();
  void Delete();
  void MoveLeft(bool selecting = false);
  void MoveRight(bool selecting = false);
  void MoveWordLeft(bool selecting = false);
  void MoveWordRight(bool selecting = false);
  void MoveUp(bool selecting = false);
  void MoveDown(bool selecting = false);
  void SetCursorPosition(std::size_t position, bool selecting = false);

  const EditorState& State() const noexcept;

 private:
  bool DeleteSelection();
  void PrepareSelection(bool selecting);
  void MoveVertically(bool upward);

  EditorState state_;
  std::optional<std::size_t> preferred_column_;
};

}
