#pragma once

#include <ftxui/dom/elements.hpp>

namespace tui_demo {

class Editor;

// Compose the sidebar, document viewport, and help pane from read-only editor state.
ftxui::Element BuildTui(const Editor& editor);

}  // namespace tui_demo
