#include "fvm_mesh.hh"
#include "ast/fmv_solver.hh"
#include "utils/csr.hh"
#include <Kokkos_Core.hpp>
#include <Kokkos_Core_fwd.hpp>
#include <cstdint>

// Helper structure to cleanly identify an edge between two vertex IDs

struct Edge {
  uint32_t v0, v1;
  Edge(uint32_t a, uint32_t b) {
    v0 = std::min(a, b);
    v1 = std::max(a, b);
  }
  bool operator<(const Edge &other) const {
    return std::tie(v0, v1) < std::tie(other.v0, other.v1);
  }
};

void FVMMesh::setup() {
  const std::vector<Vertex> &vertices = mesh.vertices;
  const std::vector<Triangle> &triangles = mesh.triangles;
  size_t num_triangles = triangles.size();

  Kokkos::View<Vector3f *, Kokkos::HostSpace> host_barycenters(
      "barycenters_host", num_triangles);

  for (uint32_t tri_id = 0; tri_id < triangles.size(); tri_id++) {
    const auto &tri = triangles[tri_id];
    const auto &p0 = vertices[tri.x()].pos;
    const auto &p1 = vertices[tri.y()].pos;
    const auto &p2 = vertices[tri.z()].pos;

    host_barycenters(tri_id) = {(p0.x() + p1.x() + p2.x()) / 3.0f,
                                (p0.y() + p1.y() + p2.y()) / 3.0f,
                                (p0.z() + p1.z() + p2.z()) / 3.0f};
  }

  // ---------------------------------------------------------
  // Step 2: Track Edge-to-Triangle Adjacency
  // ---------------------------------------------------------
  // Maps a unique edge to the triangle IDs that
  // share it
  std::map<Edge, std::vector<int>> edge_to_triangles;

  for (int i = 0; i < static_cast<int>(num_triangles); ++i) {
    const auto &tri = triangles[i];
    edge_to_triangles[Edge(tri.x(), tri.y())].push_back(i);
    edge_to_triangles[Edge(tri.y(), tri.z())].push_back(i);
    edge_to_triangles[Edge(tri.z(), tri.x())].push_back(i);
  }

  // ---------------------------------------------------------
  // Step 3: Build Intermediate Adjacency Lists
  // ---------------------------------------------------------
  // Each triangle can have up to 3 neighbors
  // sharing an edge
  std::vector<std::vector<int>> adj_lists(num_triangles);
  uint nbValues = 0;
  for (const auto &[edge, tri_ids] : edge_to_triangles) {
    // If 2 triangles share an edge, they are
    // adjacent neighbors
    if (tri_ids.size() == 2) {
      adj_lists[tri_ids[0]].push_back(tri_ids[1]);
      adj_lists[tri_ids[1]].push_back(tri_ids[0]);
      nbValues += 2;
    }
    // Note: > 2 triangles sharing an edge implies
    // a non-manifold mesh geometry
  }

  // ---------------------------------------------------------
  // Step 4: Flatten into CSR Storage format
  // ---------------------------------------------------------
  adjacency =
      CSRList<DeviceSpace, uint32_t>("Adjacency", nbValues, num_triangles + 1);

  adjacency.indices.h_view(0) = 0;
  for (int i = 0; i < static_cast<int>(num_triangles); ++i) {
    // Sort indices for strict ascending order
    // inside rows (standard for CSR algorithms)
    std::sort(adj_lists[i].begin(), adj_lists[i].end());

    adjacency.indices.h_view(i) =
        adjacency.indices.h_view(i - 1) + adj_lists[i].size();
    auto entry = adjacency.entryAt_host(i);
    const auto &cur_adj = adj_lists[i];
    for (size_t cur = 0; cur < cur_adj.size(); cur++) {
      entry(cur) = cur_adj[cur];
    }
  }
  adjacency.Sync();
  barycenters =
      Kokkos::create_mirror_view_and_copy(DeviceSpace{}, host_barycenters);
}
