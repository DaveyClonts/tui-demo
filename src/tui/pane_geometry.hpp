#pragma once
#include <algorithm>
#include <ftxui/screen/box.hpp>

namespace tui_demo {

// Screen-cell coordinates from the latest render, empty before the first render.
// Read on the UI thread. Content excludes borders and pane chrome.
struct PaneGeometry {
  ftxui::Box bounds{0, -1, 0, -1};
  ftxui::Box content{0, -1, 0, -1};
  int Width() const { return std::max(0, bounds.x_max - bounds.x_min + 1); }
  int Height() const { return std::max(0, bounds.y_max - bounds.y_min + 1); }
  int Columns() const { return std::max(0, content.x_max - content.x_min + 1); }
  int Rows() const { return std::max(0, content.y_max - content.y_min + 1); }
};

} // namespace tui_demo
