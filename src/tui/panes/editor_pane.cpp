#include "tui/panes/editor_pane.hpp"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "editor/editor.hpp"
#include "tui/assets.hpp"
#include "tui/renderer/document_renderer.hpp"

namespace tui_demo {
namespace {

class CaptureDocumentLayout : public ftxui::Node {
public:
  CaptureDocumentLayout(ftxui::Element child, DocumentLayout& layout,
                        PaneGeometry& geometry)
      : Node({std::move(child)}), layout_(layout), geometry_(geometry) {}

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
    --layout_.viewport.x_max; // Reserve the vertical scroll indicator column.
    geometry_.content = layout_.viewport;
    layout_.valid = true;
    Node::Render(screen);
  }

private:
  DocumentLayout& layout_;
  PaneGeometry& geometry_;
};

ftxui::Element RenderAsciiArt(std::string_view art) {
  ftxui::Elements lines;
  std::size_t line_start = 0;
  while (line_start < art.size()) {
    const std::size_t newline = art.find('\n', line_start);
    const std::size_t line_end =
        newline == std::string_view::npos ? art.size() : newline;
    lines.push_back(ftxui::text(
        std::string{art.substr(line_start, line_end - line_start)}));
    if (newline == std::string_view::npos) {
      break;
    }
    line_start = newline + 1;
  }
  return ftxui::vbox(std::move(lines));
}

} // namespace

std::optional<std::size_t>
DocumentLayout::PositionAt(int x, int y, std::string_view text) const {
  if (!valid || !viewport.Contain(x, y)) {
    return std::nullopt;
  }
  const int row = std::max(0, y - origin_y);
  std::size_t start = 0;
  for (int line = 0; line < row; ++line) {
    const std::size_t newline = text.find('\n', start);
    if (newline == std::string_view::npos) {
      return text.size();
    }
    start = newline + 1;
  }
  const std::size_t newline = text.find('\n', start);
  const std::size_t end = newline == std::string_view::npos ? text.size() : newline;
  const std::size_t column = static_cast<std::size_t>(std::max(0, x - origin_x));
  return start + std::min(column, end - start);
}

EditorPane::EditorPane(Editor& editor, const Keymap& keymap,
                       std::function<void()> on_edit)
    : editor_(editor), keymap_(keymap), on_edit_(std::move(on_edit)) {}

ftxui::Element EditorPane::OnRender() {
  using namespace ftxui;

  const EditorState& editor_state = editor_.State();
  document_layout_.valid = false;
  if (!Focused()) {
    input_state_.mouse_selecting = false;
  }
  // Do not render an editing cursor until a document has been opened or
  // created.
  Element document;
  if (editor_state.document_open) {
    document = vbox(RenderDocument(editor_state.currentDoc.text,
                                   editor_state.cursor_position,
                                   editor_state.selection_anchor, Focused()));
    document = std::make_shared<CaptureDocumentLayout>(
        document, document_layout_, geometry_);
    document = document | vscroll_indicator | frame | flex;
  } else {
    document = RenderAsciiArt(kScribletTitle) | center;
  }
  document = document | reflect(geometry_.content);
  Element top_pane =
      editor_state.document_open
          ? window(text(" " + editor_state.currentDoc.file.filename().string() +
                        " ") |
                       bold,
                   document) |
                yflex
          : document | border | yflex;

  return top_pane | reflect(geometry_.bounds);
}

bool EditorPane::OnEvent(ftxui::Event event) {
  if (event.is_mouse()) {
    const ftxui::Mouse& mouse = event.mouse();
    if (mouse.button == ftxui::Mouse::Left &&
        mouse.motion == ftxui::Mouse::Pressed &&
        geometry_.bounds.Contain(mouse.x, mouse.y)) {
      TakeFocus();
    }
  }
  if (!Focused()) {
    input_state_.mouse_selecting = false;
    return false;
  }
  const std::optional<std::size_t> position = event.is_mouse()
      ? document_layout_.PositionAt(event.mouse().x, event.mouse().y,
                                    editor_.State().currentDoc.text)
      : std::nullopt;
  const InputResult result = HandleInput(event, keymap_, editor_, input_state_, position);
  if (result == InputResult::Handled && on_edit_) {
    on_edit_();
  }
  return result == InputResult::Handled;
}

} // namespace tui_demo
