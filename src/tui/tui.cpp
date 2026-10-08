#include "tui/tui.hpp"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/terminal.hpp>

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "editor/editor.hpp"
#include "tui/renderer/renderer.hpp"
#include "tui/assets.hpp"

namespace tui_demo {
namespace {

class CaptureDocumentLayout : public ftxui::Node {
 public:
  CaptureDocumentLayout(ftxui::Element child, DocumentLayout& layout)
      : Node({std::move(child)}), layout_(layout) {}

  void ComputeRequirement() override {
    Node::ComputeRequirement();
    requirement_ = children_[0]->requirement();
  }

  void SetBox(ftxui::Box box) override {
    Node::SetBox(box);
    children_[0]->SetBox(box);
  }

  void Render(ftxui::Screen& screen) override {
    // reflect() clips its box; preserve the original origin for scrolled text.
    layout_.origin_x = box_.x_min;
    layout_.origin_y = box_.y_min;
    layout_.viewport = screen.stencil;
    --layout_.viewport.x_max;  // Reserve the vertical scroll indicator column.
    layout_.valid = true;
    Node::Render(screen);
  }

 private:
  DocumentLayout& layout_;
};

ftxui::Element RenderAsciiArt(std::string_view art) {
  ftxui::Elements lines;
  std::size_t line_start = 0;
  while (line_start < art.size()) {
    const auto newline = art.find('\n', line_start);
    const auto line_end = newline == std::string_view::npos ? art.size() : newline;
    lines.push_back(ftxui::text(std::string{art.substr(line_start,
                                                       line_end - line_start)}));
    if (newline == std::string_view::npos) {
      break;
    }
    line_start = newline + 1;
  }
  return ftxui::vbox(std::move(lines));
}

}

std::optional<std::size_t> DocumentLayout::PositionAt(
    int x, int y, std::string_view text) const {
  if (!valid || !viewport.Contain(x, y)) {
    return std::nullopt;
  }
  const auto row = std::max(0, y - origin_y);
  std::size_t start = 0;
  for (int line = 0; line < row; ++line) {
    const auto newline = text.find('\n', start);
    if (newline == std::string_view::npos) {
      return text.size();
    }
    start = newline + 1;
  }
  const auto newline = text.find('\n', start);
  const auto end = newline == std::string_view::npos ? text.size() : newline;
  const auto column = static_cast<std::size_t>(std::max(0, x - origin_x));
  return start + std::min(column, end - start);
}

ftxui::Element BuildTui(const Editor& editor, const std::string& status_message,
                       DocumentLayout* layout) {
  using namespace ftxui;

  // Recompute proportions on each render so the layout follows terminal resizing.
  const int bottom_pane_height =
      std::max(6, Terminal::Size().dimy * 20 / 100);

  const auto& editor_state = editor.State();
  if (layout) {
    layout->valid = false;
  }
  // Do not render an editing cursor until a document has been opened or created.
  Element document;
  if (editor_state.document_open) {
    document = vbox(RenderDocument(editor_state.currentDoc.text,
                                    editor_state.cursor_position,
                                    editor_state.selection_anchor));
    if (layout) {
      document = std::make_shared<CaptureDocumentLayout>(document, *layout);
    }
    document = document | vscroll_indicator | frame | flex;
  } else {
    document = RenderAsciiArt(kScribletTitle) | center;
  }
  auto top_pane = editor_state.document_open
      ? window(text(" " + editor_state.currentDoc.file.filename().string() + " ") |
                   bold,
               document) |
            yflex
      : document | border | yflex;

  // This is a static help pane, not an embedded shell or terminal process.
  auto bottom_pane = vbox({
                         text(" Terminal ") | bold | center,
                         separator(),
                         text("Ctrl+S: save | Esc: quit") | dim | center,
                         text(status_message) | center,
                     }) |
                     border | size(HEIGHT, EQUAL, bottom_pane_height);

  return vbox({
             top_pane,
             bottom_pane,
         }) |
         xflex | yflex;
}

}  // namespace tui_demo
