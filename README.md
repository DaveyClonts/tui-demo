# FTXUI split-screen demo

A small C++17 terminal UI with two equally sized side-by-side panes.

## Build

FTXUI is downloaded automatically when CMake configures the project.

```powershell
cmake -S . -B build
cmake --build build --config Release
```

## Run

With a single-config generator:

```powershell
.\build\ftxui_split_demo.exe
```

With Visual Studio or another multi-config generator:

```powershell
.\build\Release\ftxui_split_demo.exe
```

Press `Q` or `Esc` to exit.

