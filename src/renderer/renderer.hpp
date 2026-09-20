#pragma once

#include <ftxui/dom/elements.hpp>

#include <cstddef>
#include <optional>
#include <string_view>

namespace tui_demo {

// Build one element per line, including a final empty line after a trailing newline.
// Positions are byte offsets; selection is the half-open range between anchor and cursor.
// A missing anchor renders only the text and cursor. The document is never modified.
ftxui::Elements RenderDocument(std::string_view document,
                               std::size_t cursor_position,
                               std::optional<std::size_t> selection_anchor = std::nullopt);

}  // namespace tui_demo
