#pragma once

#include <filesystem>
#include <string>
#include <variant>

namespace tui_demo {

class Editor;

struct OpenCommand {
  std::filesystem::path path;
  // Callers must explicitly choose to discard edits when replacing a dirty document.
  bool discard_changes = false;
};

// Save the current document back to its existing path.
struct SaveCommand {};

// Add future request types here, each with its own typed arguments and handler.
// The dispatcher has no terminal dependency, so a CLI, prompt, or menu can use it.
using CommandRequest = std::variant<OpenCommand, SaveCommand>;

enum class CommandError {
  None,
  InvalidArguments,
  UnsavedChanges,
  FileAccess,
  NotRegularFile,
  ReadFailed,
  WriteFailed,
};

struct CommandResult {
  CommandError error = CommandError::None;
  std::string message;

  bool ok() const noexcept { return error == CommandError::None; }
};

class CommandDispatcher {
 public:
  explicit CommandDispatcher(Editor& editor) : editor_(editor) {}
  CommandResult Dispatch(const CommandRequest& request);

 private:
  CommandResult Execute(const OpenCommand& command);
  CommandResult Execute(const SaveCommand& command);
  Editor& editor_;
};

}  // namespace tui_demo
