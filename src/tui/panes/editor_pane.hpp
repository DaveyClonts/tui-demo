#pragma once
#include <functional>
#include <optional>
#include <string_view>
#include <ftxui/component/component_base.hpp>
#include "input/keymap.hpp"
#include "tui/panes/pane_geometry.hpp"

namespace tui_demo {
class Editor;

struct DocumentLayout {
  bool valid = false;
  ftxui::Box viewport;
  int origin_x = 0;
  int origin_y = 0;
  std::optional<std::size_t> PositionAt(int x, int y, std::string_view text) const;
};

class EditorPane : public ftxui::ComponentBase {
 public:
  // Referenced objects must outlive the component.
  EditorPane(Editor& editor, const Keymap& keymap, std::function<void()> on_edit);
  ftxui::Element OnRender() override;
  bool OnEvent(ftxui::Event event) override;
  bool Focusable() const override { return true; }
  const PaneGeometry& Geometry() const { return geometry_; }

 private:
  Editor& editor_;
  const Keymap& keymap_;
  std::function<void()> on_edit_;
  InputState input_state_;
  DocumentLayout document_layout_;
  PaneGeometry geometry_;
};
} // namespace tui_demo
