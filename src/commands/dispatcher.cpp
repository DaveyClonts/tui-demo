#include "commands/dispatcher.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <utility>

#include "editor/editor.hpp"

namespace tui_demo {
namespace {

// Own only the temporary directory created for this save, including failed saves.
struct SaveTemporaryDirectory {
  std::filesystem::path path;
  ~SaveTemporaryDirectory() {
    if (!path.empty()) {
      std::error_code ignored;
      std::filesystem::remove_all(path, ignored);
    }
  }
};

}  // namespace

CommandResult CommandDispatcher::Dispatch(const CommandRequest& request) {
  // Adding a request to the variant requires a matching Execute overload at compile time.
  return std::visit([this](const auto& command) { return Execute(command); }, request);
}

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

CommandResult CommandDispatcher::Execute(const SaveCommand&) {
  const auto& document = editor_.State().currentDoc;
  if (document.file.empty()) {
    return {CommandError::InvalidArguments, "Open a file first; this document has no path"};
  }

  std::error_code error;
  // Resolve symlinks so saving updates the target without replacing the link itself.
  const auto target = std::filesystem::canonical(document.file, error);
  if (error) {
    return {CommandError::FileAccess, "Cannot access file: " + error.message()};
  }
  const auto status = std::filesystem::status(target, error);
  if (error || !std::filesystem::is_regular_file(status)) {
    return {CommandError::FileAccess, "Cannot save to this path"};
  }
  // Check write access before using replacement, which otherwise only requires
  // permission on the containing directory. Opening without truncation preserves bytes.
  {
    std::fstream writable(target, std::ios::in | std::ios::out | std::ios::binary);
    if (!writable) {
      return {CommandError::FileAccess, "File is not writable"};
    }
  }

  // Stage beside the target so rename stays on the same filesystem. A failed
  // write/close leaves the original file intact and the editor marked modified.
  SaveTemporaryDirectory temporary;
  const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
  for (int attempt = 0; attempt < 100; ++attempt) {
    const auto candidate = target.parent_path() /
        (".edit-tui-save-" + std::to_string(stamp) + "-" + std::to_string(attempt));
    if (std::filesystem::create_directory(candidate, error)) {
      temporary.path = candidate;
      break;
    }
    if (error) {
      return {CommandError::WriteFailed, "Cannot prepare save: " + error.message()};
    }
  }
  if (temporary.path.empty()) {
    return {CommandError::WriteFailed, "Cannot create a temporary save file"};
  }
  const auto staged = temporary.path / "document";
  std::ofstream output(staged, std::ios::binary | std::ios::trunc);
  if (!output) {
    return {CommandError::WriteFailed, "Cannot open temporary save file"};
  }
  // Bound each write rather than converting the entire document size to streamsize.
  for (std::size_t offset = 0; offset < document.text.size(); offset += 8192) {
    const auto count = std::min<std::size_t>(8192, document.text.size() - offset);
    output.write(document.text.data() + offset, static_cast<std::streamsize>(count));
    if (!output) break;
  }
  output.close();
  if (!output) {
    return {CommandError::WriteFailed, "Could not write the complete file"};
  }
  std::filesystem::permissions(staged, status.permissions(), error);
  if (error) {
    return {CommandError::WriteFailed, "Cannot preserve file permissions: " + error.message()};
  }
  std::filesystem::rename(staged, target, error);
  if (error) {
    return {CommandError::WriteFailed, "Cannot replace file: " + error.message()};
  }
  editor_.MarkSaved();
  return {};
}

}  // namespace tui_demo
