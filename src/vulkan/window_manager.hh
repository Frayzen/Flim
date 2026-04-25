#pragma once

namespace Flim {
class WindowManager {
public:
  WindowManager() = default;
  ~WindowManager();
  void initWindow();
  bool framebufferResized = false;
};
}; // namespace Flim
