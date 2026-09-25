#include "commands/dispatcher.hpp"

namespace tui_demo {

CommandResult CommandDispatcher::Dispatch(const CommandRequest& request) {
  // Adding a request to the variant requires a matching Execute overload at compile time.
  return std::visit([this](const auto& command) { return Execute(command); }, request);
}

}  // namespace tui_demo
