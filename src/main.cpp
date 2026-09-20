#include "application.hpp"

// Keep process startup separate from the terminal application and editing logic.
int main() {
  return tui_demo::RunApplication();
}
