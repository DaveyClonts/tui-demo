#include "tui/panes/terminal_pane.hpp"
#include <filesystem>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <sstream>
#include <system_error>
#include <utility>

namespace tui_demo {
TerminalPane::TerminalPane(const std::string& status_message,
                           SubmitHandler submit_handler)
    : status_message_(status_message),
      submit_handler_(std::move(submit_handler)) {
  ftxui::InputOption options;
  options.multiline = false;
  options.transform = [](ftxui::InputState state) { return state.element; };
  options.on_enter = [this] {
    submit_handler_(std::exchange(input_text_, {}));
  };
  Add(ftxui::Input(&input_text_, options));
}

ftxui::Element TerminalPane::OnRender() {
  using namespace ftxui;
  Element title = text(" Terminal ") | bold;
  if (Focused()) {
    title = title | inverted;
  }
  Elements messages;
  std::istringstream message_stream(status_message_);
  std::string line;
  while (std::getline(message_stream, line)) {
    messages.push_back(text(line));
  }
  std::error_code path_error;
  const std::filesystem::path current_path =
      std::filesystem::current_path(path_error);
  const std::string prompt =
      (path_error ? "[directory unavailable]" : current_path.string()) + "> ";
  Element content = vbox({
                        hbox({text(prompt), ChildAt(0)->Render() | flex}),
                        vbox(std::move(messages)) | yframe | flex,
                    }) |
                    flex | reflect(geometry_.content);
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
  return Focused() && ChildAt(0)->OnEvent(event);
}
} // namespace tui_demo
