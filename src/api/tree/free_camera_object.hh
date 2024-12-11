#pragma once

#include "api/tree/tree_object.hh"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>
namespace Flim {

class FreeCameraObject : public TreeObject {

public:
  FreeCameraObject(TreeObject *parent)
      : TreeObject(parent), is2D(false), near(0.1f), far(10), fov(90), speed(1),
        sensivity(1), pitch(0), yaw(0), lockPitch(45) {};

  bool is2D;
  float near;
  float far;
  float fov; // in degrees

  float speed;
  float sensivity;
  float pitch, yaw, lockPitch;

  mat4 getProjMat(float screenRatio) {
    if (is2D) {
      return glm::ortho(-screenRatio, screenRatio, 1.0f, -1.0f, near, far);
    }
    return glm::perspective(glm::radians(fov), screenRatio, near, far);
  }

  mat4 getViewMat() {
    return glm::lookAt(transform.position,
                       transform.position + transform.front(), transform.up());
  }

  void handleInputs(double deltaTime);
  static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);

private:
  void handleInputs3D(double deltaTime);
  void handleInputs2D(double deltaTime);
};

} // namespace Flim
