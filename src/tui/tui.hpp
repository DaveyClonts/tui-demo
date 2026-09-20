#pragma once

#include <ftxui/dom/elements.hpp>
#include <string>

namespace tui_demo {

class Editor;

// Compose the sidebar, document viewport, and help pane from read-only editor state.
ftxui::Element BuildTui(const Editor& editor, const std::string& status_message = {});

}  // namespace tui_demo
