#include "application.hpp"

#include "tui/editor_pane.hpp"
#include "tui/terminal_pane.hpp"

namespace tui_demo {

bool Application::HandleEvent(ftxui::Event event) {
  const std::optional<Command> command = keymap_.Lookup(event);
  if (command == Command::Save) {
    if (!editor_.State().document_open) {
      return true;
    }
    const CommandResult saved = dispatcher_.Dispatch(SaveCommand{});
    status_message_ = saved.ok() ? "Saved" : "Save failed: " + saved.message;
    return true;
  }
  if (command == Command::Quit) {
    screen_.ExitLoopClosure()();
    return true;
  }
  if (command == Command::FocusNextPane) {
    if (editor_pane_->Focused()) {
      terminal_pane_->TakeFocus();
    } else {
      editor_pane_->TakeFocus();
    }
    return true;
  }
  if (command == Command::FocusPreviousPane) {
    if (terminal_pane_->Focused()) {
      editor_pane_->TakeFocus();
    } else {
      terminal_pane_->TakeFocus();
    }
    return true;
  }
  return false;
}

} // namespace tui_demo
