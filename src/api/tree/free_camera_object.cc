#include "free_camera_object.hh"
#include "api/flim_api.hh"
#include "api/scene.hh"
#include <GLFW/glfw3.h>
#include <glm/ext/quaternion_transform.hpp>
#include <iostream>

namespace Flim {

void CameraObject::handleInputs2D(double deltaTime) {
  (void)deltaTime;

  /* float zoom = -transform.position.x; */
  /* bool canZoom = zoom > transform.minZoom; */
  /* bool canUnzoom = zoom < transform.maxZoom; */

  /* auto adaptSpeed = speed * (0.2f + 2 * (zoom - minZoom) / (maxZoom -
   * minZoom)); */
  /* if (glfwGetKey(win_, GLFW_KEY_D) == GLFW_PRESS) */
  /*   position += adaptSpeed * getRight(); */
  /* if (glfwGetKey(win_, GLFW_KEY_A) == GLFW_PRESS) */
  /*   position += adaptSpeed * -getRight(); */
  /* if (glfwGetKey(win_, GLFW_KEY_W) == GLFW_PRESS) */
  /*   position += adaptSpeed * up_; */
  /* if (glfwGetKey(win_, GLFW_KEY_S) == GLFW_PRESS) */
  /*   position += adaptSpeed * -up_; */
  /* if (glfwGetKey(win_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && canUnzoom) */
  /*   position += speed * 3 * -getFront(); */
  /* if (glfwGetKey(win_, GLFW_KEY_SPACE) == GLFW_PRESS && canZoom) */
  /*   position += speed * 3 * getFront(); */
}

void CameraObject::handleInputs3D(double deltaTime) {
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
  pitch = max(min(pitch, lockPitch), -lockPitch);
  transform.rotation = glm::normalize(
      glm::quat(glm::vec3(glm::radians(pitch), glm::radians(yaw), 0.0f)));
}

void CameraObject::handleInputs(double deltaTime) {
  if (is2D)
    handleInputs2D(deltaTime);
  else
    handleInputs3D(deltaTime);
}

} // namespace Flim
