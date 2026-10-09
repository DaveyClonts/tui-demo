#include "tui/terminal_pane.hpp"
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

namespace tui_demo {
TerminalPane::TerminalPane(const std::string& status_message)
    : status_message_(status_message) {}

ftxui::Element TerminalPane::OnRender() {
  using namespace ftxui;
  Element title = text(" Terminal ") | bold;
  if (Focused()) {
    title = title | inverted;
  }
  Element content = vbox({
      text("Terminal not connected") | dim | center,
      text(status_message_) | center,
      filler(),
  }) | flex | reflect(geometry_.content);
  return window(title, content) | reflect(geometry_.bounds);
}

bool TerminalPane::OnEvent(ftxui::Event event) {
  if (event.is_mouse()) {
    const ftxui::Mouse& mouse = event.mouse();
    if (mouse.button == ftxui::Mouse::Left &&
        mouse.motion == ftxui::Mouse::Pressed &&
        geometry_.bounds.Contain(mouse.x, mouse.y)) {
      TakeFocus();
      return true;
    }
    return false;
  }
  return Focused();
}
} // namespace tui_demo
