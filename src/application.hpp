#pragma once

#include <filesystem>
#include <optional>

namespace tui_demo {

// Own the editor and fullscreen terminal session until the event loop exits.
int RunApplication(std::optional<std::filesystem::path> initial_path = std::nullopt);

}  // namespace tui_demo
