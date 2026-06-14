#pragma once

#include "api/mesh/mesh.hh"
#include "utils/csr.hh"
#include <Eigen/Core>
#include <Kokkos_DualView.hpp>

using DeviceSpace = Kokkos::DefaultHostExecutionSpace;

using namespace Flim;

class FVMMesh {
public:
  FVMMesh(const Mesh &m) : mesh(m) { setup(); };

  const Mesh &mesh;
  const auto &getAdjacency() const { return adjacency; }
  const auto &getBarycenters() const { return barycenters; }

private:
  CSRList<DeviceSpace, uint32_t> adjacency;
  Kokkos::View<Vector3f *, DeviceSpace> barycenters;

  void setup();
};
