#pragma once

#include "api/tree/tree_object.hh"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>
namespace Flim {

class CameraObject : public TreeObject {

public:
  CameraObject(TreeObject *parent)
      : TreeObject(parent), is2D(false), near(0.1f), far(10), fov(90) {};

  bool is2D;
  float near;
  float far;
  float fov;

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
};

} // namespace Flim
