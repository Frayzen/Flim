#pragma once

#include "fwd.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_common.hpp>

namespace Flim {

const vec3 world_front = vec3(0, 0, -1);
const vec3 world_right = vec3(1, 0, 0);
const vec3 world_up = vec3(0, 1, 0);

class Transform {

public:
  Transform()
      : position({0, 0, 0}), rotation(glm::identity<quat>()),
        scale(vec3(1, 1, 1)) {};
  vec3 position;
  quat rotation;
  vec3 scale;

  // Right hand rule applies
  vec3 front() const {
    // The forward vector in world space (-Z)
    return glm::normalize(rotation * world_front);
  }
  vec3 up() const {
    // The forward vector in world space (+Y)
    return glm::normalize(rotation * world_up);
  }
  vec3 right() const {
    // The forward vector in world space (+X)
    return glm::normalize(rotation * world_right);
  }

  void translate(vec3 v) { position = position + v; }

  glm::mat4 getViewMatrix() const {
    glm::mat4 m = glm::translate(glm::identity<mat4>(), position);
    glm::mat4 rot = glm::mat4_cast(glm::conjugate(rotation));
    m = glm::scale(rot * m, scale);
    return m;
  };
};

const Transform world = Flim::Transform();

} // namespace Flim
