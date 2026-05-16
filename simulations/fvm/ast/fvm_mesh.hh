#pragma once

#include "api/render/mesh.hh"
#include "ast/fmv_solver.hh"
#include "utils/csr.hh"
#include <Eigen/Core>
#include <Kokkos_DualView.hpp>

using namespace Flim;

class FVMMesh {
public:
  FVMMesh(Mesh m) : mesh(m) { setup(); };

private:
  Mesh mesh;
  CSRList<DeviceSpace, uint32_t> adjacency;
  Kokkos::View<Vector3f *, DeviceSpace> barycenters;

  void setup();
};
