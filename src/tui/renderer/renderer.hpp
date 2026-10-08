#pragma once

#include <ftxui/dom/elements.hpp>

#include <cstddef>
#include <optional>
#include <string_view>

namespace tui_demo {

ftxui::Elements
RenderDocument(std::string_view document, std::size_t cursor_position,
               std::optional<std::size_t> selection_anchor = std::nullopt);

} // namespace tui_demo
