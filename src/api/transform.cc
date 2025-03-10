#include "transform.hh"
#include "utils/geometry.hh"
#include <Eigen/src/Core/Matrix.h>
#include <Eigen/src/Geometry/Quaternion.h>

namespace Flim {

// Right hand rule applies
Vector3f Transform::front() const {
  // The forward vector in world space (-Z)
  return (rotation * world_front).normalized();
}
Vector3f Transform::up() const {
  // The forward vector in world space (+Y)
  return (rotation * world_up).normalized();
}
Vector3f Transform::right() const {
  // The forward vector in world space (+X)
  return (rotation * world_right).normalized();
}

void Transform::translate(Vector3f v) { position = position + v; }

void Transform::lookAt(Vector3f target) {
  rotation = lookToward(position - target);
}

Matrix4f Transform::getViewMatrix() const {
  // Create a translation matrix
  auto translation = Eigen::Translation3f(position);
  // Create a rotation matrix
  auto rotationMatrix = rotation.toRotationMatrix();
  // Create a scale matrix
  auto scaleMatrix = Eigen::Scaling(scale);

  // Apply transformations in the same order (translate, rotate, scale)
  return (translation * rotationMatrix * scaleMatrix).matrix();
};

} // namespace Flim
