# FTXUI split-screen
 
A small C++17 terminal editor built with FTXUI.

## Build

FTXUI is downloaded automatically when CMake configures the project.

```powershell
cmake -S . -B build
cmake --build build --config Release
```

## Run

With a single-config generator:

```powershell
./build/edit_tui
```

With Visual Studio or another multi-config generator:

```powershell
.\build\Release\edit_tui.exe
```

Open an existing file on startup (quote paths containing spaces):

```sh
./build/edit_tui open "notes/my document.md"
```

`--help` prints usage. Invalid arguments exit with code 2; file-loading errors
print a message and exit with code 1 before entering the terminal UI. Files are
read into memory without modifying them. Missing files are reported rather than
created. Ctrl+S saves changes back to the opened file.

- Arrow keys move the cursor.
- Shift+Arrow keys extend or shrink the highlighted selection, including across lines.
- Left/Right without Shift collapse a selection to its start/end.
- Backspace/Delete remove the selection; otherwise they delete before/at the cursor.
- Typing or Enter replaces the selection.
- Ctrl+S saves the opened file and displays success or an error in the bottom pane.
- Esc exits.

Shift+Arrow requires a terminal that forwards those keys to the application.
Cursor positions currently use byte offsets, so editing multibyte Unicode is not yet supported.

## Key bindings

`src/input/keymap.cpp` defines the default keys and dispatches named commands to
the editor. `application.cpp` forwards input and handles the quit result.
To customize keys in code, configure the `Keymap` after it is constructed:

```cpp
keymap.Unbind(ftxui::Event::ArrowLeft);
keymap.Bind(ftxui::Event::Character('h'), tui_demo::Command::MoveLeft);
```

`Bind` replaces any existing action on that key. Omit `Unbind` to keep the old key
as an additional shortcut. Bound characters invoke their command; unbound
characters insert text. Configuration file loading and a remapping UI are not
implemented yet. The help pane currently describes the default bindings.

## Document commands

`src/commands/dispatcher.hpp` exposes commands independently of keyboard events
and the terminal UI. The initial document command is `OpenCommand`:

```cpp
tui_demo::CommandDispatcher dispatcher(editor);
const auto result = dispatcher.Dispatch(tui_demo::OpenCommand{"notes.md"});
if (!result.ok()) {
  // Inspect result.error or display result.message in the calling UI.
}
```

Successful open replaces the document and resets the cursor, selection, and
preferred column. It accepts regular files (including symlinks to them), reads
their bytes synchronously, and preserves the current editor state if loading
fails. Relative paths resolve from the process working directory; paths are
literal filesystem paths, with no shell expansion inside the dispatcher.

Edits mark the document modified. Opening another file then returns
`CommandError::UnsavedChanges`. A caller that has explicitly decided to discard
those edits can pass `OpenCommand{path, true}`. No-op edits and cursor movement
do not mark the document modified. Undo is not implemented yet.

`dispatcher.Dispatch(SaveCommand{})` saves to the current document path. A successful
save clears the modified flag without moving the cursor or selection. Writes are
staged beside the original file before replacement, preserving file permissions
and following symlinks to their targets. Saving requires write access to the file
and its parent directory. Replacement does not preserve hard-link identity or
all platform-specific metadata. An untitled document reports an error; Save As
and a filename prompt are not implemented yet.

To add a document command, define its request struct, add it to `CommandRequest`,
and implement an `Execute` overload in `CommandDispatcher`. The variant dispatch
requires every request type to have a handler at compile time. Future command
prompts or menus can call the same API; keyboard editing actions still use the
existing keymap dispatcher. File loading currently runs on the calling thread,
so large or slow files would need background loading in a future interactive UI.
