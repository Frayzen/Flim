#pragma once

#include "api/transform.hh"
#include "vulkan/rendering/utils.hh"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // for perspective projection
#include <fwd.hh>
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

  Matrix4f getProjMat(float screenRatio) const {
    Eigen::Matrix4f proj = Eigen::Matrix4f::Identity();

    if (is2D) {
      // Orthographic projection
      float left = -screenRatio;
      float right = screenRatio;
      float bottom = -1.0f;
      float top = 1.0f;

      proj(0, 0) = 2.0f / (right - left);
      proj(1, 1) = 2.0f / (top - bottom);
      proj(2, 2) = 2.0f / (far - near);
      proj(0, 3) = -(right + left) / (right - left);
      proj(1, 3) = -(top + bottom) / (top - bottom);
      proj(2, 3) = -(far + near) / (far - near);
    } else {
      // Perspective projection
      float tanHalfFov =
          std::tan(TO_RAD(fov)); // Convert fov to radians

      proj(0, 0) = 1.0f / (screenRatio * tanHalfFov);
      proj(1, 1) = -1.0f / tanHalfFov;
      proj(2, 2) = -(far + near) / (far - near);
      proj(2, 3) = -(2.0f * far * near) / (far - near);
      proj(3, 2) = -1.0f;
      proj(3, 3) = 0.0f;
    }

    return proj;
  }

  Matrix4f getViewMat() const {
    // Create a translation matrix
    auto translation = Eigen::Translation3f(-transform.position);
    // Create a rotation matrix
    auto rotationMatrix = toQuaternion(0.0f, -TO_RAD(pitch), -TO_RAD(yaw));
    // Create a scale matrix
    auto scaleMatrix = Eigen::Scaling(transform.scale);

    // Apply transformations in the same order as glm (translate, rotate, scale)
    return (scaleMatrix * rotationMatrix * translation).matrix();
  }

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
