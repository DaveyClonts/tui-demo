#include "cli/commands/dispatcher.hpp"

#include <filesystem>
#include <fstream>
#include <string>

#include "editor/editor.hpp"

namespace tui_demo {
namespace {

CommandResult FindAvailablePath(std::filesystem::path& new_path) {
  std::error_code error;
  const auto directory = std::filesystem::current_path(error);
  if (error) {
    return {CommandError::FileAccess,
            "Cannot access the current directory: " + error.message()};
  }

  for (int suffix = 0; suffix < 100; ++suffix) {
    const auto filename = suffix == 0
        ? std::string{"newFile.md"}
        : "newFile" + std::to_string(suffix) + ".md";
    const auto candidate = directory / filename;
    const bool exists = std::filesystem::exists(candidate, error);
    if (error) {
      return {CommandError::FileAccess,
              "Cannot inspect the current directory: " + error.message()};
    }
    if (!exists) {
      new_path = candidate;
      return {};
    }
  }

  return {CommandError::WriteFailed,
          "Cannot find an available new Markdown filename"};
}

}  // namespace

CommandResult CommandDispatcher::Execute(const NewCommand&) {
  if (editor_.State().modified) {
    return {CommandError::UnsavedChanges,
            "The current document has unsaved changes"};
  }

  std::filesystem::path new_path;
  auto result = FindAvailablePath(new_path);
  if (!result.ok()) return result;

  std::ofstream file(new_path, std::ios::binary);
  if (!file) {
    return {CommandError::WriteFailed,
            "Cannot create the new Markdown file"};
  }
  file.close();
  if (!file) {
    return {CommandError::WriteFailed,
            "Cannot finish creating the new Markdown file"};
  }

  // Store the new path in editor state so Ctrl+S writes back to this file.
  editor_.LoadDocument(MarkdownFile{new_path, {}});
  return {};
}

}  // namespace tui_demo
