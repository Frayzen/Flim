#pragma once

#include "api/transform.hh"
#include <Eigen/Eigen>
#include <Eigen/src/Core/Matrix.h>
#include <Eigen/src/Geometry/AngleAxis.h>
#include <Eigen/src/Geometry/Quaternion.h>
#include <cmath>

using namespace Eigen;
namespace Flim {

inline Eigen::Quaternionf lookToward(const Eigen::Vector3f &direction,
                                     const Eigen::Vector3f &up = world_front) {
  Vector3f dir = direction.normalized();
  Vector3f newup = up.normalized();
  if (dir == newup)
    return Quaternionf::FromTwoVectors(dir, world_front).inverse();
  return Quaternionf::FromTwoVectors(dir, newup).inverse();
}

} // namespace Flim
