#pragma once
#include <string>
#include <ftxui/component/component_base.hpp>
#include "tui/pane_geometry.hpp"

namespace tui_demo {
// Interactive placeholder for the future PTY. No shell is started yet.
class TerminalPane : public ftxui::ComponentBase {
 public:
  // status_message must outlive the component.
  explicit TerminalPane(const std::string& status_message);
  ftxui::Element OnRender() override;
  bool OnEvent(ftxui::Event event) override;
  bool Focusable() const override { return true; }
  const PaneGeometry& Geometry() const { return geometry_; }

 private:
  const std::string& status_message_;
  PaneGeometry geometry_;
};
} // namespace tui_demo
