#pragma once

#include <ftxui/component/event.hpp>

#include <optional>
#include <vector>

namespace tui_demo {

class Editor;

// Commands describe intent independently of which key invokes them.
enum class Command {
  MoveLeft,
  MoveRight,
  MoveUp,
  MoveDown,
  SelectLeft,
  SelectRight,
  SelectUp,
  SelectDown,
  Backspace,
  DeleteForward,
  InsertNewline,
  Save,
  Quit,
};

struct KeyBinding {
  ftxui::Event key;
  Command command;
};

class Keymap {
 public:
  // Start with the application's default controls.
  Keymap();

  // Each key has at most one command. Binding an existing key replaces its action;
  // multiple keys may invoke the same command. Unbind removes the old key on remap.
  void Bind(ftxui::Event key, Command command);
  void Unbind(const ftxui::Event& key);
  std::optional<Command> Lookup(const ftxui::Event& key) const;

 private:
  std::vector<KeyBinding> bindings_;
};

// Let the application own terminal shutdown, while input dispatch stays testable.
enum class InputResult { Unhandled, Handled, Quit, Save };

InputResult ExecuteCommand(Command command, Editor& editor);

// Explicit bindings take precedence over ordinary character insertion.
InputResult HandleInput(const ftxui::Event& event, const Keymap& keymap,
                        Editor& editor);

}  // namespace tui_demo
