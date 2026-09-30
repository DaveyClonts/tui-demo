#pragma once

#include <ftxui/dom/elements.hpp>
#include <string>
#include <optional>
#include <string_view>
#include <ftxui/screen/box.hpp>

namespace tui_demo {

class Editor;

struct DocumentLayout {
  bool valid = false;
  ftxui::Box viewport;
  int origin_x = 0;
  int origin_y = 0;

  std::optional<std::size_t> PositionAt(int x, int y, std::string_view text) const;
};

// Compose the sidebar, document viewport, and help pane from read-only editor state.
ftxui::Element BuildTui(const Editor& editor, const std::string& status_message = {},
                       DocumentLayout* layout = nullptr);

}  // namespace tui_demo
