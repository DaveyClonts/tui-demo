#include "tui/tui.hpp"

#include <algorithm>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/screen/terminal.hpp>

#include <cstddef>
#include <memory>
#include <utility>

namespace tui_demo {
namespace {

// A focus container without FTXUI's built-in key bindings. Focus changes only
// when a child calls TakeFocus(), allowing the application keymap to own every
// keyboard shortcut.
class PaneContainer final : public ftxui::ComponentBase {
 public:
  PaneContainer(ftxui::Component editor_pane, ftxui::Component terminal_pane)
  {
    Add(std::move(editor_pane));
    Add(std::move(terminal_pane));
  }

  ftxui::Element OnRender() override {
    using namespace ftxui;
    const int terminal_height = std::max(6, Terminal::Size().dimy / 5);
    return vbox({
        ChildAt(0)->Render() | yflex,
        ChildAt(1)->Render() | size(HEIGHT, EQUAL, terminal_height),
    }) | flex;
  }

  bool OnEvent(ftxui::Event event) override {
    if (event.is_mouse()) {
      for (ftxui::Component& child : children()) {
        if (child->OnEvent(event)) {
          return true;
        }
      }
      return false;
    }
    const ftxui::Component active_child = ActiveChild();
    return Focused() && active_child && active_child->OnEvent(event);
  }

  ftxui::Component ActiveChild() override {
    return ChildAt(active_child_index_);
  }

  void SetActiveChild(ftxui::ComponentBase* child) override {
    for (std::size_t index = 0; index < ChildCount(); ++index) {
      if (ChildAt(index).get() == child) {
        active_child_index_ = index;
        return;
      }
    }
  }

  bool Focusable() const override { return true; }

 private:
  std::size_t active_child_index_ = 0;
};

} // namespace

ftxui::Component BuildTui(ftxui::Component editor_pane,
                          ftxui::Component terminal_pane) {
  return std::make_shared<PaneContainer>(std::move(editor_pane),
                                         std::move(terminal_pane));
}
} // namespace tui_demo
