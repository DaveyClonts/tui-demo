# FTXUI split-screen demo

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

- Arrow keys move the cursor.
- Shift+Arrow keys extend or shrink the highlighted selection, including across lines.
- Left/Right without Shift collapse a selection to its start/end.
- Backspace/Delete remove the selection; otherwise they delete before/at the cursor.
- Typing or Enter replaces the selection.
- Esc exits.

Shift+Arrow requires a terminal that forwards those keys to the application.
Cursor positions currently use byte offsets, so editing multibyte Unicode is not yet supported.

Run the editor and rendering checks with `ctest --test-dir build --output-on-failure`.

