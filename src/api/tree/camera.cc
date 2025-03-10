#include "camera.hh"
#include "api/flim_api.hh"
#include "api/scene.hh"
#include "vulkan/rendering/utils.hh"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <fwd.hh>

namespace Flim {

void Camera::handleInputs2D(double deltaTime) {
  auto curSpeed = deltaTime * speed;
  auto win = scene.api.getWindow();
  if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
    transform.position += curSpeed * world_right;
  if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
    transform.position += curSpeed * -world_right;
  if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
    transform.position += curSpeed * world_up;
  if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
    transform.position += curSpeed * -world_up;
  if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    orthoScale += curSpeed;
  if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS)
    orthoScale -= curSpeed;
  orthoScale = std::clamp(orthoScale, minOrthoScale, maxOrthoScale);
}

void Camera::handleInputs3D(double deltaTime) {
  auto win = scene.api.getWindow();
  float curSpeed = speed * deltaTime;
  if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
    transform.position += curSpeed * transform.right();
  if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
    transform.position += curSpeed * -transform.right();
  if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
    transform.position += curSpeed * transform.front();
  if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
    transform.position += curSpeed * -transform.front();
  if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    transform.position += curSpeed * -world_up;
  if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS)
    transform.position += curSpeed * world_up;

  float curSensivity = sensivity;
  if (glfwGetKey(win, GLFW_KEY_L) == GLFW_PRESS)
    yaw -= curSensivity;
  if (glfwGetKey(win, GLFW_KEY_H) == GLFW_PRESS)
    yaw += curSensivity;
  if (glfwGetKey(win, GLFW_KEY_J) == GLFW_PRESS)
    pitch -= curSensivity;
  if (glfwGetKey(win, GLFW_KEY_K) == GLFW_PRESS)
    pitch += curSensivity;
  pitch = std::max(std::min(pitch, lockPitch), -lockPitch);

  // Combine yaw and pitch (order matters: typically Yaw * Pitch)
  transform.rotation = toQuaternion(0.0f, TO_RAD(pitch), TO_RAD(yaw));
}

void Camera::handleInputs(double deltaTime) {
  if (is2D)
    handleInputs2D(deltaTime);
  else
    handleInputs3D(deltaTime);
}

Matrix4f Camera::getViewMat() const {
  // Create a translation matrix
  auto translation = Eigen::Translation3f(-transform.position);
  // Create a rotation matrix
  auto rotationMatrix = toQuaternion(0.0f, -TO_RAD(pitch), -TO_RAD(yaw));
  // Create a scale matrix
  auto scaleMatrix = Eigen::Scaling(transform.scale);

  // Apply transformations in the same order as (translate, rotate, scale)
  return (scaleMatrix * rotationMatrix * translation).matrix();
}

Matrix4f Camera::getProjMat(float screenRatio) const {

  if (is2D) {
    Eigen::Matrix4f ortho = Eigen::Matrix4f::Identity();
    // Orthographic projection
    float left = -screenRatio * orthoScale;
    float right = screenRatio * orthoScale;
    // inverted to match opengl y axis
    float bottom = 1.0f * orthoScale;
    float top = -1.0f * orthoScale;

    ortho(0, 0) = 2.0f / (right - left);
    ortho(1, 1) = 2.0f / (top - bottom);
    ortho(2, 2) = -2.0f / (far - near);
    ortho(0, 3) = -(right + left) / (right - left);
    ortho(1, 3) = -(top + bottom) / (top - bottom);
    ortho(2, 3) = -(near) / (far - near);
    return ortho;
  } else {
    Eigen::Matrix4f proj = Eigen::Matrix4f::Identity();
    // Perspective projection
    float tanHalfFov = std::tan(TO_RAD(fov)); // Convert fov to radians

    proj(0, 0) = 1.0f / (screenRatio * tanHalfFov);
    proj(1, 1) = -1.0f / tanHalfFov;
    proj(2, 2) = -(far + near) / (far - near);
    proj(2, 3) = -(2.0f * far * near) / (far - near);
    proj(3, 2) = -1.0f;
    proj(3, 3) = 0.0f;

    return proj;
  }
}

} // namespace Flim
