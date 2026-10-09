# FTXUI split-screen
 
A small C++20 terminal editor built with FTXUI.

## Build

FTXUI and CLI11 are downloaded automatically when CMake configures the project.

```powershell
cmake -S . -B build
cmake --build build --config Release
```

## Run

With a single-config generator:

```powershell
./build/edit_tui
```

Starting without a subcommand opens the application with no document. Editing
and navigation remain disabled until a document is opened or created.

With Visual Studio or another multi-config generator:

```powershell
.\build\Release\edit_tui.exe
```

Open an existing file on startup (quote paths containing spaces):

```sh
./build/edit_tui open "notes/my document.md"
```

Create and open a new Markdown file in the current directory:

```sh
./build/edit_tui new
```

The first file is named `newFile.md`; if that name exists, the editor tries
`newFile1.md`, `newFile2.md`, and so on.

`--help` (or `-h`) prints usage; `open --help` describes the file argument.
Startup argument parsing lives in `src/cli/commands/cli.cpp`.
Invalid arguments exit with code 2; file-loading errors
print a message and exit with code 1 before entering the terminal UI. Files are
read into memory without modifying them. Missing files are reported rather than
created. Ctrl+S saves changes back to the opened file.

- Arrow keys move the cursor.
- Left-click in the document to position the cursor; Shift+click extends the
  selection. Hold the left button and drag to select text across lines; releasing
  anywhere ends the drag. Dragging outside the document keeps the last selection
  until you return or release. Clicks past a line end stop at that line's end, and clicks below
  the text go to the end of the document. Mouse positioning currently assumes
  one terminal column per byte (plain ASCII without tabs).
- Ctrl+Left/Right jump to the previous/next word start, crossing spaces, tabs,
  and newlines. Words are runs of non-whitespace characters, including punctuation.
  With a selection, they collapse it to its start/end.
- Shift+Arrow keys extend or shrink the highlighted selection, including across lines.
- Left/Right without Shift collapse a selection to its start/end.
- Backspace/Delete remove the selection; otherwise they delete before/at the cursor.
- Typing or Enter replaces the selection.
- Ctrl+S saves the opened file and displays success or an error in the bottom pane.
- Esc exits.
- Tab/Shift+Tab switch between the editor and terminal panes; clicking a pane
  also focuses it. The terminal is currently a placeholder and starts no PTY.

Shift+Arrow requires a terminal that forwards those keys to the application.
Cursor positions currently use byte offsets, so editing multibyte Unicode is not yet supported.

## Key bindings

`src/editor/input/keymap.cpp` defines every default keyboard shortcut. `EditorPane`
handles document commands; `application.cpp` handles focus, save, and quit
commands globally, regardless of the focused pane. FTXUI does not assign pane
navigation keys.
To customize keys in code, configure the `Keymap` after it is constructed:

```cpp
keymap.Unbind(ftxui::Event::ArrowLeft);
keymap.Bind(ftxui::Event::Character('h'), tui_demo::Command::MoveLeft);
keymap.Unbind(ftxui::Event::Tab);
keymap.Bind(ftxui::Event::CtrlN, tui_demo::Command::FocusNextPane);
```

`Bind` replaces any existing action on that key. Omit `Unbind` to keep the old key
as an additional shortcut. Bound characters invoke their command; unbound
characters insert text. Configuration file loading and a remapping UI are not
implemented yet. The help pane currently describes the default bindings.

## Pane components and geometry

`tui/tui.cpp` composes persistent `EditorPane` and `TerminalPane` components.
`EditorPane` owns document hit-testing and delegates text drawing to
`document_renderer`. The application retains the editor and pane instances.

Both panes expose `const PaneGeometry& Geometry() const`:

- `bounds`, `Width()`, and `Height()` describe the entire pane.
- `content`, `Columns()`, and `Rows()` describe its usable content area,
  excluding borders and, for an open document, the scrollbar column.

Coordinates are inclusive screen-cell bounds. Geometry is empty before the
first render and refreshed during every render, including after a resize.
Read it on the UI thread after rendering; retain a copy if another thread
needs a snapshot. A future PTY resize can use
`terminal_pane->Geometry().Columns()` and `Rows()`.

## Document commands

`src/cli/commands/dispatcher.hpp` exposes commands independently of keyboard events
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
