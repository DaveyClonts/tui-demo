#pragma once

#include <ftxui/component/component_base.hpp>

namespace tui_demo {

// Compose persistent panes. Each pane exposes its own latest rendered geometry.
ftxui::Component BuildTui(ftxui::Component editor_pane,
                          ftxui::Component terminal_pane);

}  // namespace tui_demo
