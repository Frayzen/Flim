#pragma once

#include "api/fwd.hh"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

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
  template <typename T>
  std::shared_ptr<const T> child(size_t i) const {
    assert(children.size() > i);
    return std::dynamic_pointer_cast<const T>(children[i]);
  }
  size_t nbChildren() const { return children.size(); }

  virtual ~TreeObject() = default;

protected:
  TreeObject(TreeObject *parent)
      : parent(parent), scene(parent->scene) {};

  TreeObject(Scene& scene)
      : parent(nullptr), scene(scene) {};
  TreeObject *parent;
  std::vector<std::shared_ptr<TreeObject>> children;
  friend class InstanceObject;
};

} // namespace Flim
