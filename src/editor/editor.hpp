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
  MarkdownFile currentDoc;
};

class Editor {
 public:
  void Insert(std::string_view text);
  void InsertNewline();
  void Backspace();
  void MoveLeft();
  void MoveRight();
  void MoveUp();
  void MoveDown();

  const EditorState& State() const noexcept;

 private:
  void MoveVertically(bool upward);

  EditorState state_;
  std::optional<std::size_t> preferred_column_; //when moving from a longer line to a shorter line
};

}  // namespace tui_demo
