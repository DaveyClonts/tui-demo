#pragma once
#include <functional>
#include <string>
#include <ftxui/component/component_base.hpp>
#include "tui/pane_geometry.hpp"

namespace tui_demo {
// Collects application commands; submission is handled by the application.
class TerminalPane : public ftxui::ComponentBase {
 public:
  // status_message must outlive the component.
  using SubmitHandler = std::function<void(std::string)>;
  TerminalPane(const std::string& status_message, SubmitHandler submit_handler);
  ftxui::Element OnRender() override;
  bool OnEvent(ftxui::Event event) override;
  bool Focusable() const override { return true; }
  const PaneGeometry& Geometry() const { return geometry_; }

 private:
  const std::string& status_message_;
  std::string input_text_;
  SubmitHandler submit_handler_;
  PaneGeometry geometry_;
};
} // namespace tui_demo
