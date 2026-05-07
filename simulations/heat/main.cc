#include "api/flim_api.hh"
#include "api/parameters/base_params.hh"
#include "api/parameters/render_params.hh"
#include "api/render/mesh.hh"
#include "api/render/mesh_utils.hh"
#include "api/tree/instance.hh"
#include "kokkos/renderer_accesser.hh"
#include "vulkan/buffers/params_utils.hh"
#include "vulkan/rendering/renderer.hh"
#include <Eigen/src/Core/Matrix.h>
#include <HIP/Kokkos_HIP_Parallel_Team.hpp>
#include <KokkosKernels_default_types.hpp>
#include <KokkosSparse_CrsMatrix.hpp>
#include <KokkosSparse_spmv.hpp>
#include <Kokkos_Core.hpp>
#include <Kokkos_DualView.hpp>
#include <Kokkos_Macros.hpp>
#include <Kokkos_Pair.hpp>
#include <cmath>
#include <cstdint>
#include <decl/Kokkos_Declare_OPENMP.hpp>
#include <imgui.h>
#include <impl/Kokkos_Profiling.hpp>
#include <setup/Kokkos_Setup_HIP.hpp>

using namespace Flim;

#include <KokkosBlas1_axpby.hpp>
#include <KokkosBlas1_dot.hpp>
#include <KokkosSparse_CrsMatrix.hpp>
#include <KokkosSparse_spmv.hpp>

// Define types for clarity
using DeviceSpace = Kokkos::DefaultExecutionSpace;
using MatrixType = KokkosSparse::CrsMatrix<float, int, DeviceSpace, void, int>;

// --- CORRECTED CG SOLVER FOR SPARSE ---
Kokkos::View<float *> solve_CG_sparse(MatrixType A, Kokkos::View<float *> f,
                                      int size) {
  Kokkos::View<float *> u("u", size);
  Kokkos::View<float *> r("r", size);
  Kokkos::View<float *> p("p", size);
  Kokkos::View<float *> Ap("Ap", size);

  Kokkos::deep_copy(u, 0.0f);
  Kokkos::deep_copy(r, f);
  Kokkos::deep_copy(p, r);

  float rr_old = KokkosBlas::dot(r, r);

  for (int iter = 0; iter < 1000; iter++) {
    // Sparse Matrix-Vector Multiply: Ap = 1.0 * A * p + 0.0 * Ap
    KokkosSparse::spmv("N", 1.0f, A, p, 0.0f, Ap);

    float pAp = KokkosBlas::dot(p, Ap);
    if (std::abs(pAp) < 1e-12f)
      break;

    float alpha = rr_old / pAp;
    KokkosBlas::axpy(alpha, p, u);
    KokkosBlas::axpy(-alpha, Ap, r);

    float rr_new = KokkosBlas::dot(r, r);
    if (sqrt(rr_new) < 1e-6f)
      break;

    float beta = rr_new / rr_old;
    KokkosBlas::axpby(1.0f, r, beta, p);
    rr_old = rr_new;
  }
  return u;
}
int main() {
  Kokkos::initialize();
  FlimAPI api = FlimAPI::init();
  {

    float side_length = 1;
    float diag_length = sqrt(2) * side_length;
    int nb_x = 25;
    int nb_y = 15;

    Mesh mesh = MeshUtils::createGrid(side_length, nb_x, nb_y);
    auto &scene = api.getScene();
    RenderParams params("Default");

    // Uniforms
    ParamsUtils::createViewMatrixUniform(params, BINDING_DEFAULT_VIEWS_UNIFORM,
                                         mesh, scene.camera);
    ParamsUtils::createInstanceMatrixAttribute(params, 2);
    ParamsUtils::createVerticesAttribute(params, 0, true, false, false);
    params.vertexShader = Shader("shaders/default.vert.spv");
    params.fragmentShader = Shader("shaders/default.frag.spv");
    static Vector4f color(1.0f, 0.0f, 0.0f, 1.0f);
    params.setAttribute(1, AttributeRate::VERTEX)
        .attach<Vector4f>([](const Mesh &m, Vector4f *colors) {
          for (uint i = 0; i < m.getVertices().size(); i++)
            colors[i] = color;
        })
        .add(0, VK_FORMAT_R32G32B32A32_SFLOAT)
        .computeFriendly(true)
        .singleBuffered(true)
        .onlySetup(true);
    params.useBackfaceCulling = false;
    params.mode = RenderMode::RENDERER_MODE_LINE;
    params.fragmentShader = Shader("shaders/fvm.frag.spv");
    params.vertexShader = Shader("shaders/fvm.vert.spv");
    const Renderer &rd = scene.registerMesh(mesh, params);
    Instance &c = scene.instantiate(mesh);

    api.setupGraphics();

    auto colors = getAttributeBufferView<Vector4f>(rd, 1);

    scene.camera.controls = true;
    scene.camera.speed = 5;
    scene.camera.sensivity = 5;

    bool running = false;

    int total = nb_x * nb_y;

    // 1. Determine number of non-zeros (nnz)
    // Each interior node has 5 entries (self + 4 neighbors)
    // This is an upper bound for simplicity
    int approx_nnz = total * 5;

    // 2. Create the StaticCrsGraph or use the constructor that takes row counts
    // For simplicity, we use the manual entry method here:
    typename MatrixType::StaticCrsGraphType::row_map_type::non_const_type
        row_map("row_map", total + 1);

    // Count entries per row (Host-side setup is often easier for stencils)
    auto row_map_host = Kokkos::create_mirror_view(row_map);
    int nnz_count = 0;
    for (int j = 0; j < nb_y; j++) {
      for (int i = 0; i < nb_x; i++) {
        row_map_host(j * nb_x + i) = nnz_count;
        if (j == 0 || j == nb_y - 1)
          nnz_count += 1; // Boundary
        else {
          int count = 1; // diagonal
          if (i > 0)
            count++;
          if (i < nb_x - 1)
            count++;
          if (j > 0)
            count++;
          if (j < nb_y - 1)
            count++;
          nnz_count += count;
        }
      }
    }
    row_map_host(total) = nnz_count;
    Kokkos::deep_copy(row_map, row_map_host);

    typename MatrixType::index_type::non_const_type entries("entries",
                                                            nnz_count);
    typename MatrixType::values_type::non_const_type values("values",
                                                            nnz_count);

    MatrixType A("PoissonMatrix", total, total, nnz_count, values, row_map,
                 entries);

    // 3. Fill the matrix (Parallel)
    Kokkos::View<float *> f("f", total);
    Kokkos::parallel_for(
        "FillSparseSymmetric", total, KOKKOS_LAMBDA(const int idx) {
          int i = idx % nb_x;
          int j = idx / nb_x;
          int row_start = row_map(idx);
          int entry_ptr = row_start;

          if (j == 0) {
            // Boundary: Top (100)
            entries(entry_ptr) = idx;
            values(entry_ptr) = 1.0f;
            f(idx) = 100.0f;
          } else if (j == nb_y - 1) {
            // Boundary: Bottom (0)
            entries(entry_ptr) = idx;
            values(entry_ptr) = 1.0f;
            f(idx) = 0.0f;
          } else {
            // Interior Diagonal
            entries(entry_ptr) = idx;
            float diag_val = 0.0f;
            float rhs_val = 0.0f;

            auto handle_neighbor = [&](int ni, int nj) {
              int n_idx = nj * nb_x + ni;
              if (nj == 0) {
                // Neighbor is a boundary!
                // Subtract (-1.0 * boundary_value) from RHS
                rhs_val -= (-1.0f * 100.0f);
              } else if (nj == nb_y - 1) {
                // Boundary is 0, so RHS contribution is 0
                rhs_val -= (-1.0f * 0.0f);
              } else {
                // Normal interior neighbor: add to matrix
                entry_ptr++;
                entries(entry_ptr) = n_idx;
                values(entry_ptr) = -1.0f;
              }
              diag_val += 1.0f;
            };

            if (i > 0)
              handle_neighbor(i - 1, j);
            if (i < nb_x - 1)
              handle_neighbor(i + 1, j);
            if (j > 0)
              handle_neighbor(i, j - 1);
            if (j < nb_y - 1)
              handle_neighbor(i, j + 1);

            values(row_start) = diag_val;
            f(idx) = rhs_val;
          }
        });

    auto solution = solve_CG_sparse(A, f, total);

    // Debug: Print some solution values
    auto solution_host =
        Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), solution);
    printf("\nSolution sample (bottom to top):\n");
    for (int j = 0; j < nb_y; j++) {
      int i = nb_x / 2; // Middle column
      int idx = j * nb_x + i;
      printf("j=%d (row %d): %f\n", j, idx / nb_x, solution_host(idx));
    }

    float scale = 0.01f;
    bool set = false;
    float cold[3] = {0, 0, 1.0};
    float hot[3] = {1.0, 0, 0};
    api.run([&](float deltaTime) {
      (void)deltaTime;
      const char *items[] = {"Triangles", "Bars", "Dots"};
      if (ImGui::Combo("Rendering type", ((int *)&(params.mode)), items,
                       IM_ARRAYSIZE(items))) {
        params.invalidate();
      }
      // if (ImGui::SliderFloat3("Color", &color.x(), 0, 1)) {
      ImGui::Checkbox("Set ", &set);
      ImGui::SliderFloat("Scale ", &scale, 0.01, 100);

      ImGui::ColorPicker3("Cold", cold, ImGuiColorEditFlags_InputRGB);
      ImGui::ColorPicker3("Hot", hot, ImGuiColorEditFlags_InputRGB);

      float scale_v = scale;
      bool set_v = set;

      Kokkos::parallel_for(
          "Apply color", colors.size(), KOKKOS_LAMBDA(const uint32_t id) {
            // colors[id] = toApply;
            float v = scale_v;
            if (!set_v)
              v *= solution(id);
            Vector4f coldVec(cold[0], cold[1], cold[2], 1.0f);
            Vector4f hotVec(hot[0], hot[1], hot[2], 1.0f);
            colors[id] = coldVec * (1 - v) + v * hotVec;
          });

      ImGui::Checkbox("Running", &running);
      if (ImGui::Button("Step") || running) {
      }
    });
  }
  Kokkos::finalize();
  return 0;
}
