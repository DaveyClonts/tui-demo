#pragma once

#include "commands/cli.hpp"

namespace tui_demo {

// Own the editor and fullscreen terminal session until the event loop exits.
int RunApplication(const StartupOptions& options = {});

}  // namespace tui_demo
