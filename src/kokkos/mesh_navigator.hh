#pragma once

#include "api/mesh/mesh.hh"
#include "kokkos/utils/CSR.hh"
#include <Eigen/Core>
#include <Kokkos_Core.hpp>
#include <cstdint>
class MeshNavigator {
  using IndexType = uint32_t;

  struct Edge {
    IndexType node1;
    IndexType node2;
  };

public:
  MeshNavigator(const Flim::Mesh &m) : mesh(m) { setup(); };

  void setup();

  CSRList<Edge, Kokkos::DefaultExecutionSpace> cells;
  CSRList<Edge, Kokkos::DefaultExecutionSpace>
      faces; // ame as edges for 2D mesh
  Kokkos::View<Edge *, Kokkos::DefaultExecutionSpace> edges;
  Kokkos::View<Eigen::Vector3f *, Kokkos::DefaultExecutionSpace> nodes;

  const Flim::Mesh &mesh;
};
