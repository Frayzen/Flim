#include "camera.hh"
#include "api/flim_api.hh"
#include "api/scene.hh"
#include "vulkan/rendering/utils.hh"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <fwd.hh>

namespace Flim {

void Camera::handleInputs2D(double deltaTime) {
  (void)deltaTime;

  float zoom = -transform.position.x();
  bool canZoom = zoom > minZoom2D;
  bool canUnzoom = zoom < maxZoom2D;

  auto adaptSpeed =
      speed * (0.2f + 2 * (zoom - minZoom2D) / (maxZoom2D - minZoom2D));
  auto win = scene.api.getWindow();
  if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
    transform.position += adaptSpeed * world.right();
  if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
    transform.position += adaptSpeed * -world.right();
  if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
    transform.position += adaptSpeed * world.up();
  if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
    transform.position += adaptSpeed * world.up();
  if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && canUnzoom)
    transform.position += speed * 3 * -world.front();
  if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS && canZoom)
    transform.position += speed * 3 * world.front();
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
    transform.position += curSpeed * -world.up();
  if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS)
    transform.position += curSpeed * world.up();

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
  transform.rotation = toQuaternion(Vector3f(TO_RAD(yaw), TO_RAD(pitch), 0));
}

void Camera::handleInputs(double deltaTime) {
  if (is2D)
    handleInputs2D(deltaTime);
  else
    handleInputs3D(deltaTime);
}

} // namespace Flim
