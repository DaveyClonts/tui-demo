#include "commands/dispatcher.hpp"

#include <array>
#include <fstream>
#include <utility>

#include "editor/editor.hpp"

namespace tui_demo {

CommandResult CommandDispatcher::Execute(const OpenCommand& command) {
  if (command.path.empty() ||
      command.path.native().find(std::filesystem::path::value_type{}) !=
          std::filesystem::path::string_type::npos) {
    return {CommandError::InvalidArguments, "open requires a nonempty path without NUL bytes"};
  }
  if (editor_.State().modified && !command.discard_changes) {
    return {CommandError::UnsavedChanges, "The current document has unsaved changes"};
  }

  std::error_code error;
  const auto status = std::filesystem::status(command.path, error);
  if (error) {
    return {CommandError::FileAccess, "Cannot access file: " + error.message()};
  }
  // Reject directories and special devices rather than attempting to read a stream
  // that may never end. Symlinks to regular files are accepted by status().
  if (!std::filesystem::is_regular_file(status)) {
    return {CommandError::NotRegularFile, "The path must refer to an existing regular file"};
  }

  std::ifstream input(command.path, std::ios::binary);
  if (!input) {
    return {CommandError::FileAccess, "Cannot open file for reading"};
  }

  MarkdownFile document{command.path, {}};
  std::array<char, 8192> buffer;
  // Stage all bytes before changing the editor. An empty file is a valid document;
  // a read error must not leave the editor with a partially loaded file.
  while (input.read(buffer.data(), buffer.size()) || input.gcount() > 0) {
    document.text.append(buffer.data(), static_cast<std::size_t>(input.gcount()));
  }
  if (input.bad() || !input.eof()) {
    return {CommandError::ReadFailed, "Could not read the complete file"};
  }

  editor_.LoadDocument(std::move(document));
  return {};
}

}  // namespace tui_demo
