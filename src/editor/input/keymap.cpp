#include "editor/input/keymap.hpp"

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
      // Modifier 5 means Ctrl in these terminal sequences.
      {Event::Special("\x1B[1;5D"), Command::MoveWordLeft},
      {Event::Special("\x1B[1;5C"), Command::MoveWordRight},
      // Modifier 6 means Ctrl+Shift.
      {Event::Special("\x1B[1;6D"), Command::SelectWordLeft},
      {Event::Special("\x1B[1;6C"), Command::SelectWordRight},
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
  bindings_.erase(std::remove_if(bindings_.begin(), bindings_.end(),
                                 [&](const KeyBinding& binding) {
                                   return binding.key == key;
                                 }),
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
  case Command::MoveLeft:
    editor.MoveLeft();
    break;
  case Command::MoveRight:
    editor.MoveRight();
    break;
  case Command::MoveWordLeft:
    editor.MoveWordLeft();
    break;
  case Command::MoveWordRight:
    editor.MoveWordRight();
    break;
  case Command::SelectWordLeft:
    editor.MoveWordLeft(true);
    break;
  case Command::SelectWordRight:
    editor.MoveWordRight(true);
    break;
  case Command::MoveUp:
    editor.MoveUp();
    break;
  case Command::MoveDown:
    editor.MoveDown();
    break;
  case Command::SelectLeft:
    editor.MoveLeft(true);
    break;
  case Command::SelectRight:
    editor.MoveRight(true);
    break;
  case Command::SelectUp:
    editor.MoveUp(true);
    break;
  case Command::SelectDown:
    editor.MoveDown(true);
    break;
  case Command::Backspace:
    editor.Backspace();
    break;
  case Command::DeleteForward:
    editor.Delete();
    break;
  case Command::InsertNewline:
    editor.InsertNewline();
    break;
  case Command::Save:
    return InputResult::Save;
  case Command::Quit:
    return InputResult::Quit;
  }
  return InputResult::Handled;
}

InputResult HandleInput(ftxui::Event event, const Keymap& keymap,
                        Editor& editor, InputState& input_state,
                        std::optional<std::size_t> mouse_position) {
  const auto command = keymap.Lookup(event);
  if (!editor.State().document_open) {
    input_state.mouse_selecting = false;
    if (command && *command == Command::Quit) {
      return ExecuteCommand(*command, editor);
    }
    return command || event.is_character() ? InputResult::Handled
                                           : InputResult::Unhandled;
  }

  if (command) {
    input_state.mouse_selecting = false;
    return ExecuteCommand(*command, editor);
  }
  if (event.is_mouse()) {
    const auto& mouse = event.mouse();
    if (mouse.motion == ftxui::Mouse::Released &&
        (mouse.button == ftxui::Mouse::Left ||
         mouse.button == ftxui::Mouse::None)) {
      const bool was_selecting = input_state.mouse_selecting;
      input_state.mouse_selecting = false;
      return was_selecting ? InputResult::Handled : InputResult::Unhandled;
    }
    if (mouse.button == ftxui::Mouse::Left) {
      if (mouse.motion == ftxui::Mouse::Pressed) {
        input_state.mouse_selecting = mouse_position.has_value();
        if (mouse_position) {
          editor.SetCursorPosition(*mouse_position, mouse.shift);
          return InputResult::Handled;
        }
      } else if (mouse.motion == ftxui::Mouse::Moved &&
                 input_state.mouse_selecting) {
        if (mouse_position &&
            *mouse_position != editor.State().cursor_position) {
          editor.SetCursorPosition(*mouse_position, true);
        }
        return InputResult::Handled;
      }
    }
    return InputResult::Unhandled;
  }
  if (event.is_character()) {
    input_state.mouse_selecting = false;
    editor.Insert(event.character());
    return InputResult::Handled;
  }
  return InputResult::Unhandled;
}

} // namespace tui_demo
