#pragma once

#include <ftxui/dom/elements.hpp>

namespace tui_demo {

class Editor;

ftxui::Element BuildTui(const Editor& editor);

}  // namespace tui_demo
