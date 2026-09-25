#include "commands/dispatcher.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>

#include "editor/editor.hpp"

namespace tui_demo {
namespace {

struct SaveTarget {
  std::filesystem::path path;
  std::filesystem::perms permissions;
};

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

CommandResult ResolveSaveTarget(const std::filesystem::path& document_path,
                                SaveTarget& target) {
  if (document_path.empty()) {
    return {CommandError::InvalidArguments, "Open a file first; this document has no path"};
  }

  std::error_code error;
  // Resolve symlinks so saving updates the target without replacing the link itself.
  target.path = std::filesystem::canonical(document_path, error);
  if (error) {
    return {CommandError::FileAccess, "Cannot access file: " + error.message()};
  }
  const auto status = std::filesystem::status(target.path, error);
  if (error || !std::filesystem::is_regular_file(status)) {
    return {CommandError::FileAccess, "Cannot save to this path"};
  }

  // Replacement only needs access to the containing directory. Check the file too
  // so a read-only document is not silently replaced with a writable copy.
  std::fstream writable(target.path, std::ios::in | std::ios::out | std::ios::binary);
  if (!writable) {
    return {CommandError::FileAccess, "File is not writable"};
  }

  target.permissions = status.permissions();
  return {};
}

CommandResult CreateSaveTemporaryDirectory(const std::filesystem::path& target,
                                           SaveTemporaryDirectory& temporary) {                                           
  // AUDIT: Will need better way of handling...
  // if 100 collisions happen something else is wrong
  std::error_code error;
  const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
  for (int attempt = 0; attempt < 100; ++attempt) {
    const auto candidate = target.parent_path() /
        (".edit-tui-save-" + std::to_string(stamp) + "-" + std::to_string(attempt));
    if (std::filesystem::create_directory(candidate, error)) {
      temporary.path = candidate;
      return {};
    }
    if (error) {
      return {CommandError::WriteFailed, "Cannot prepare save: " + error.message()};
    }
  }
  return {CommandError::WriteFailed, "Cannot create a temporary save file"};
}

CommandResult WriteStagedDocument(const std::filesystem::path& staged,
                                  const std::string& text,
                                  std::filesystem::perms permissions) {
  std::ofstream output(staged, std::ios::binary | std::ios::trunc);
  if (!output) {
    return {CommandError::WriteFailed, "Cannot open temporary save file"};
  }

  // Bound each write rather than converting the entire document size to streamsize.
  for (std::size_t offset = 0; offset < text.size(); offset += 8192) {
    const auto count = std::min<std::size_t>(8192, text.size() - offset);
    output.write(text.data() + offset, static_cast<std::streamsize>(count));
    if (!output) break;
  }
  output.close();
  if (!output) {
    return {CommandError::WriteFailed, "Could not write the complete file"};
  }

  std::error_code error;
  std::filesystem::permissions(staged, permissions, error);
  if (error) {
    return {CommandError::WriteFailed, "Cannot preserve file permissions: " + error.message()};
  }
  return {};
}

CommandResult ReplaceSaveTarget(const std::filesystem::path& staged,
                                const std::filesystem::path& target) {
  std::error_code error;
  std::filesystem::rename(staged, target, error);
  if (error) {
    return {CommandError::WriteFailed, "Cannot replace file: " + error.message()};
  }
  return {};
}

}  // namespace

CommandResult CommandDispatcher::Execute(const SaveCommand&) {
  const auto& document = editor_.State().currentDoc;
  SaveTarget target;
  auto result = ResolveSaveTarget(document.file, target);
  if (!result.ok()) return result;

  // Stage beside the target so rename stays on the same filesystem. A failed
  // write/close leaves the original file intact and the editor marked modified.
  SaveTemporaryDirectory temporary;
  result = CreateSaveTemporaryDirectory(target.path, temporary);
  if (!result.ok()) return result;

  const auto staged = temporary.path / "document";
  result = WriteStagedDocument(staged, document.text, target.permissions);
  if (!result.ok()) return result;

  result = ReplaceSaveTarget(staged, target.path);
  if (!result.ok()) return result;

  editor_.MarkSaved();
  return {};
}

}  // namespace tui_demo
