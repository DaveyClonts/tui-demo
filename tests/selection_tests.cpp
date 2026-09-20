#include "editor/editor.hpp"
#include "renderer/renderer.hpp"

#include <ftxui/screen/screen.hpp>

#include <cstdlib>
#include <iostream>

// Keep checks active in Release builds too, where assert may be compiled out.
void Check(bool condition, const char* message) {
  if (!condition) {
    std::cerr << message << '\n';
    std::exit(1);
  }
}

int main() {
  using tui_demo::Editor;
  // Start with a backward selection, then replace a range spanning a newline.
  Editor editor;
  editor.Insert("abc\ndef");
  editor.MoveLeft(true);
  editor.MoveLeft(true);
  editor.Backspace();
  Check(editor.State().currentDoc.text == "abc\nd", "Backward selection deletion");
  Check(editor.State().cursor_position == 5 && !editor.State().selection_anchor,
        "Deletion clears selection and places cursor at start");

  editor.MoveUp(true);
  editor.Insert("X");
  Check(editor.State().currentDoc.text == "aX", "Typing replaces multiline selection");
  editor.MoveLeft(true);
  editor.InsertNewline();
  Check(editor.State().currentDoc.text == "a\n", "Enter replaces selection");
  editor.MoveLeft();
  editor.Delete();
  Check(editor.State().currentDoc.text == "a", "Delete joins lines");
  editor.Delete();
  Check(editor.State().currentDoc.text == "a", "Delete at EOF is harmless");

  // Exercise the opposite selection direction and the start-of-document boundary.
  Editor forward;
  forward.Insert("abc");
  forward.MoveLeft();
  forward.MoveLeft();
  forward.MoveLeft();
  forward.MoveRight(true);
  forward.MoveRight(true);
  forward.Delete();
  Check(forward.State().currentDoc.text == "c" && forward.State().cursor_position == 0,
        "Forward selection deletion");
  forward.Backspace();
  Check(forward.State().currentDoc.text == "c", "Backspace at start is harmless");

  // Plain arrows collapse to endpoints; returning to the anchor selects nothing.
  Editor collapse;
  collapse.Insert("abcd");
  collapse.MoveLeft(true);
  collapse.MoveLeft(true);
  collapse.MoveRight();
  Check(collapse.State().cursor_position == 4 && !collapse.State().selection_anchor,
        "Right collapses to selection end");
  collapse.MoveLeft(true);
  collapse.MoveLeft(true);
  collapse.MoveLeft();
  Check(collapse.State().cursor_position == 2 && !collapse.State().selection_anchor,
        "Left collapses to selection start");
  collapse.MoveRight(true);
  collapse.MoveLeft(true);
  collapse.Delete();
  Check(collapse.State().currentDoc.text == "abd", "Empty selection uses forward delete");

  // Moving through a short line must preserve both the anchor and desired column.
  Editor vertical;
  vertical.Insert("abcd\nx\nabcdef");
  vertical.MoveUp(true);
  Check(vertical.State().cursor_position == 6, "Vertical selection clamps column");
  vertical.MoveUp(true);
  Check(vertical.State().cursor_position == 4, "Vertical selection retains preferred column");
  vertical.MoveDown(true);
  vertical.MoveDown(true);
  Check(vertical.State().cursor_position == 13 && vertical.State().selection_anchor == 13,
        "Selection can shrink back to anchor");

  // Inspect actual rendered cell styles, including the display-only newline cells.
  // The range [1, 5) selects b, two newlines, and c, but excludes a and d.
  auto screen = ftxui::Screen(10, 3);
  ftxui::Render(screen, ftxui::vbox(tui_demo::RenderDocument("ab\n\ncd", 5, 1)));
  Check(screen.CellAt(0, 0).background_color != ftxui::Color::Blue,
        "Unselected prefix has no highlight");
  Check(screen.CellAt(1, 0).background_color == ftxui::Color::Blue &&
        screen.CellAt(2, 0).background_color == ftxui::Color::Blue &&
        screen.CellAt(0, 1).background_color == ftxui::Color::Blue &&
        screen.CellAt(0, 2).background_color == ftxui::Color::Blue,
        "Selection highlights text, newlines, and empty lines");
  Check(screen.CellAt(1, 2).background_color != ftxui::Color::Blue,
        "Selection end is exclusive");
  auto reverse = ftxui::Screen(10, 3);
  ftxui::Render(reverse, ftxui::vbox(tui_demo::RenderDocument("ab\n\ncd", 1, 5)));
  Check(reverse.CellAt(1, 0).background_color == ftxui::Color::Blue,
        "Cursor within backward selection retains highlight");
}
