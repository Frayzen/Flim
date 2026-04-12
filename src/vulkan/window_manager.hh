#pragma once

class WindowManager {
public:
  WindowManager() = default;
  ~WindowManager();
  void initWindow();
  bool framebufferResized = false;
};
