#pragma once

#include <cstddef>
#include <filesystem>
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

  const EditorState& State() const noexcept;

 private:
  EditorState state_;
};

}  // namespace tui_demo
