#pragma once

#include "api/fwd.hh"
#include <cstdlib>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Flim {

class Transform {

public:
  Transform()
      : position({0, 0, 0}), rotation({quat(1, 0, 0, 0)}),
        scale(vec3(1, 1, 1)) {};
  vec3 position;
  quat rotation;
  vec3 scale;

  vec3 front();
  vec3 up();
  vec3 right();

  glm::mat4 getViewMatrix() {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rot = glm::mat4_cast(rotation);
    m = glm::scale(rot * m, scale);
    return m;
  };
};

class TreeObject {
public:
  Scene &scene;
  Transform transform;

protected:
  TreeObject(TreeObject *parent, Scene &scene) : parent(parent), scene(scene) {
    if (parent != nullptr)
      parent->children.push_back(this);
  };
  TreeObject *parent;
  std::vector<TreeObject *> children;
};

} // namespace Flim
