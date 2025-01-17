#pragma once

#include "api/transform.hh"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // for perspective projection
#include <fwd.hh>
namespace Flim {

extern float curcamproj;

class Scene;

class Camera {

public:
  Scene &scene;
  Transform transform;

  bool is2D;

  float scale;

  float near;
  float far;
  float fov; // in degrees

  float speed;
  float sensivity;
  float pitch, yaw, lockPitch;

  Matrix4f getProjMat(float screenRatio) const;
  Matrix4f getViewMat() const;

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
