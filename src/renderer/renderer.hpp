#pragma once

#include <ftxui/dom/elements.hpp>

#include <cstddef>
#include <string_view>

namespace tui_demo {

// Renders the document text and its cursor as terminal UI elements.
ftxui::Elements RenderDocument(std::string_view document,
                               std::size_t cursor_position);

}  // namespace tui_demo
