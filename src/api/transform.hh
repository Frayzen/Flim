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
  Vector3f front() const;
  Vector3f up() const;
  Vector3f right() const;
  void translate(Vector3f v);

  void lookAt(Vector3f target);

  Matrix4f getViewMatrix() const;
};

} // namespace Flim
