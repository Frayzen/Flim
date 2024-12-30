#pragma once

#include <fwd.hh>

namespace Flim {

const Vector3f world_front = Vector3f(0, 0, -1);
const Vector3f world_right = Vector3f(1, 0, 0);
const Vector3f world_up = Vector3f(0, 1, 0);

class Transform {

public:
  Transform()
      : position({0, 0, 0}), rotation(Quaternionf::Identity()),
        scale(Vector3f(1, 1, 1)) {};
  Vector3f position;
  Quaternionf rotation;
  Vector3f scale;

  // Right hand rule applies
  Vector3f front() const {
    // The forward vector in world space (-Z)
    return (rotation * world_front).normalized();
  }
  Vector3f up() const {
    // The forward vector in world space (+Y)
    return (rotation * world_up).normalized();
  }
  Vector3f right() const {
    // The forward vector in world space (+X)
    return (rotation * world_right).normalized();
  }

  void translate(Vector3f v) { position = position + v; }

  Matrix4f getViewMatrix() const {
    // Create a translation matrix
    auto translation = Eigen::Translation3f(position);
    // Create a rotation matrix
    auto rotationMatrix = Eigen::Affine3f(rotation);
    // Create a scale matrix
    auto scaleMatrix = Eigen::Scaling(scale);

    // Apply transformations in the same order as glm (translate, rotate, scale)
    return (scaleMatrix * rotationMatrix * translation).matrix();
  };
};

const Transform world = Flim::Transform();

} // namespace Flim
