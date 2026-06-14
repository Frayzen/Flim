#include "mesh_navigator.hh"
#include "api/mesh/mesh.hh"
#include <Kokkos_Core.hpp>
#include <decl/Kokkos_Declare_OPENMP.hpp>
#include <setup/Kokkos_Setup_HIP.hpp>

void MeshNavigator::setup() {
  Kokkos::View<const Flim::Vertex *, Kokkos::HostSpace,
               Kokkos::MemoryTraits<Kokkos::Unmanaged>>
      meshVertices(mesh.getVertices().data(), mesh.getVertices().size());
  auto deviceMeshVertices = Kokkos::create_mirror_view_and_copy(
      Kokkos::DefaultExecutionSpace{}, meshVertices);
  nodes = Kokkos::View<Eigen::Vector3f *, Kokkos::DefaultExecutionSpace>(
      "Nodes", deviceMeshVertices.size());
  auto nodesj = nodes;
  Kokkos::parallel_for(
      deviceMeshVertices.size(), KOKKOS_LAMBDA(uint32_t index) {
        nodesj(index) = deviceMeshVertices(index).pos;
      });
}
