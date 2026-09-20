#include "input/keymap.hpp"

#include <algorithm>
#include <utility>

#include "editor/editor.hpp"

namespace tui_demo {

Keymap::Keymap() {
  using ftxui::Event;
  bindings_ = {
      {Event::ArrowLeft, Command::MoveLeft},
      {Event::ArrowRight, Command::MoveRight},
      {Event::ArrowUp, Command::MoveUp},
      {Event::ArrowDown, Command::MoveDown},
      // FTXUI 7.0.3 has no named Shift+Arrow constants. Modifier 2 means Shift;
      // A/B/C/D identify Up/Down/Right/Left in these terminal sequences.
      {Event::Special("\x1B[1;2D"), Command::SelectLeft},
      {Event::Special("\x1B[1;2C"), Command::SelectRight},
      {Event::Special("\x1B[1;2A"), Command::SelectUp},
      {Event::Special("\x1B[1;2B"), Command::SelectDown},
      {Event::Backspace, Command::Backspace},
      {Event::Delete, Command::DeleteForward},
      {Event::Return, Command::InsertNewline},
      {Event::CtrlS, Command::Save},
      {Event::Escape, Command::Quit},
  };
}

void Keymap::Bind(ftxui::Event key, Command command) {
  for (auto& binding : bindings_) {
    if (binding.key == key) {
      binding.command = command;
      return;
    }
  }
  bindings_.push_back({std::move(key), command});
}

void Keymap::Unbind(const ftxui::Event& key) {
  bindings_.erase(
      std::remove_if(bindings_.begin(), bindings_.end(),
                     [&](const KeyBinding& binding) { return binding.key == key; }),
      bindings_.end());
}

std::optional<Command> Keymap::Lookup(const ftxui::Event& key) const {
  // The small binding list can compare FTXUI events directly without hashing.
  for (const auto& binding : bindings_) {
    if (binding.key == key) {
      return binding.command;
    }
  }
  return std::nullopt;
}

InputResult ExecuteCommand(Command command, Editor& editor) {
  switch (command) {
    case Command::MoveLeft: editor.MoveLeft(); break;
    case Command::MoveRight: editor.MoveRight(); break;
    case Command::MoveUp: editor.MoveUp(); break;
    case Command::MoveDown: editor.MoveDown(); break;
    case Command::SelectLeft: editor.MoveLeft(true); break;
    case Command::SelectRight: editor.MoveRight(true); break;
    case Command::SelectUp: editor.MoveUp(true); break;
    case Command::SelectDown: editor.MoveDown(true); break;
    case Command::Backspace: editor.Backspace(); break;
    case Command::DeleteForward: editor.Delete(); break;
    case Command::InsertNewline: editor.InsertNewline(); break;
    case Command::Save: return InputResult::Save;
    case Command::Quit: return InputResult::Quit;
  }
  return InputResult::Handled;
}

InputResult HandleInput(const ftxui::Event& event, const Keymap& keymap,
                        Editor& editor) {
  if (const auto command = keymap.Lookup(event)) {
    return ExecuteCommand(*command, editor);
  }
  if (event.is_character()) {
    editor.Insert(event.character());
    return InputResult::Handled;
  }
  return InputResult::Unhandled;
}

}  // namespace tui_demo
