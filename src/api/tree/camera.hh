#pragma once

#include "api/transform.hh"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // for perspective projection
#include <fwd.hh>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/trigonometric.hpp>
namespace Flim {

class Scene;

class Camera {

public:
  Scene &scene;
  Transform transform;

  bool is2D;
  float minZoom2D = 1.0f;
  float maxZoom2D = 10.0f;

  float near;
  float far;
  float fov; // in degrees

  float speed;
  float sensivity;
  float pitch, yaw, lockPitch;

  mat4 getProjMat(float screenRatio) const {
    if (is2D) {
      return glm::ortho(-screenRatio, screenRatio, 1.0f, -1.0f, near, far);
    }
    auto proj = glm::perspective(glm::radians(fov), screenRatio, near, far);
    proj[1][1] *= -1; // because y component in glsl is opposite
    return proj;
  }

  mat4 getViewMat() const { return transform.getViewMatrix(); }

  void handleInputs(double deltaTime);
  static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);

private:
  Camera(Scene &scene)
      : scene(scene), is2D(false), near(0.1f), far(1000), fov(90), speed(1),
        sensivity(1), pitch(0), yaw(0), lockPitch(45) {};
  void handleInputs3D(double deltaTime);
  void handleInputs2D(double deltaTime);
  friend Scene;
};

} // namespace Flim
