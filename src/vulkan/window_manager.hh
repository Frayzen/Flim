#pragma once

class WindowManager {
public:
  WindowManager() = default;
  void initWindow();
  bool framebufferResized = false;
};
