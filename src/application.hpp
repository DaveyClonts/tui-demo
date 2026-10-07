#pragma once

#include "cli/commands/cli.hpp"

namespace tui_demo {

// Own the editor and fullscreen terminal session until the event loop exits.
int RunApplication(const StartupRequest& request = {});

}  // namespace tui_demo
