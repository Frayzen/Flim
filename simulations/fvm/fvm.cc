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

// --- CG SOLVER FOR SPARSE ---
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
    KokkosSparse::spmv("N", 1.0f, A, p, 0.0f, Ap);

    float pAp = KokkosBlas::dot(p, Ap);
    if (std::abs(pAp) < 1e-12f)
      break;

    float alpha = rr_old / pAp;
    KokkosBlas::axpy(alpha, p, u);
    KokkosBlas::axpy(-alpha, Ap, r);

    float rr_new = KokkosBlas::dot(r, r);
    if (std::sqrt(rr_new) < 1e-6f)
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
    // Simulation parameters
    const int nb_x = 25;
    const int nb_y = 15;
    const int total = nb_x * nb_y;

    const float L_x = 1.0f;
    const float L_y = 1.0f;
    const float dx = L_x / (nb_x - 1);
    const float dy = L_y / (nb_y - 1);

    // Equation coefficients
    const float dt = 0.01f;
    const float T_d = 1.0f;   // Time coefficient
    const float A_d = 0.1f;   // Advection coefficient
    const float B_d = 0.05f;  // Diffusion coefficient
    const float K = 0.01f;    // Reaction coefficient
    const float Vj = dx * dy; // Control volume area (2D)

    Mesh mesh = MeshUtils::createGrid(L_x, nb_x, nb_y);
    auto &scene = api.getScene();
    RenderParams params("Default");

    ParamsUtils::createViewMatrixUniform(params, BINDING_DEFAULT_VIEWS_UNIFORM,
                                         mesh, scene.camera);
    ParamsUtils::createInstanceMatrixAttribute(params, 2);
    ParamsUtils::createVerticesAttribute(params, 0, true, false, false);

    params.vertexShader = Shader("shaders/fvm.vert.spv");
    params.fragmentShader = Shader("shaders/fvm.frag.spv");
    params.setAttribute(1, AttributeRate::VERTEX)
        .attach<Vector4f>([](const Mesh &m, Vector4f *colors) {
          for (uint i = 0; i < m.getVertices().size(); i++)
            colors[i] = Vector4f(1, 1, 1, 1);
        })
        .add(0, VK_FORMAT_R32G32B32A32_SFLOAT)
        .computeFriendly(true)
        .singleBuffered(true)
        .onlySetup(true);

    params.mode = RenderMode::RENDERER_MODE_LINE;
    const Renderer &rd = scene.registerMesh(mesh, params);
    scene.instantiate(mesh);

    api.setupGraphics();
    // auto colors = getAttributeBufferView<Vector4f>(rd, 1);

    // --- MATRIX TOPOLOGY SETUP ---
    typename MatrixType::StaticCrsGraphType::row_map_type::non_const_type
        row_map("row_map", total + 1);
    auto row_map_host = Kokkos::create_mirror_view(row_map);

    int nnz_count = 0;
    for (int idx = 0; idx < total; idx++) {
      row_map_host(idx) = nnz_count;
      int i = idx % nb_x;
      int j = idx / nb_x;
      nnz_count++; // Diagonal
      if (i > 0)
        nnz_count++;
      if (i < nb_x - 1)
        nnz_count++;
      if (j > 0)
        nnz_count++;
      if (j < nb_y - 1)
        nnz_count++;
    }
    row_map_host(total) = nnz_count;
    Kokkos::deep_copy(row_map, row_map_host);

    typename MatrixType::index_type::non_const_type entries("entries",
                                                            nnz_count);
    typename MatrixType::values_type::non_const_type values("values",
                                                            nnz_count);
    MatrixType A("FVMMatrix", total, total, nnz_count, values, row_map,
                 entries);

    // Simulation State
    Kokkos::View<float *> u_n("u_n", total);     // Solution at time n
    Kokkos::View<float *> f_ext("f_ext", total); // Source term
    Kokkos::View<float *> b("rhs", total);       // Right hand side

    // Init: High value at center
    Kokkos::parallel_for(
        total, KOKKOS_LAMBDA(int i) {
          u_n(i) = 0.0f;
          f_ext(i) = 0.0f;
        });

    bool running = false;
    float cold[3] = {0.1f, 0.1f, 0.5f};
    float hot[3] = {1.0f, 0.2f, 0.1f};

    api.run([&](float deltaTime) {
      (void)deltaTime;

      if (running) {
        // --- ASSEMBLE MATRIX AND RHS ---
        Kokkos::parallel_for(
            "Assemble", total, KOKKOS_LAMBDA(const int j_idx) {
              int i = j_idx % nb_x;
              int j = j_idx / nb_x;
              int row_start = row_map(j_idx);
              int entry_ptr = row_start;

              // 1. Diagonal: Vi * (Td/dt + K)
              float diag_val = Vj * (T_d / dt) + Vj * K;
              // 2. RHS: Vj * (f + (Td/dt)*u_n)
              float rhs_val = Vj * f_ext(j_idx) + Vj * (T_d / dt) * u_n(j_idx);

              entries(entry_ptr) = j_idx; // diag is first in our row_map logic
              entry_ptr++;

              auto add_face = [&](int ni, int nj, float n_d, float face_len,
                                  float dist) {
                int k_idx = nj * nb_x + ni;
                // Term: |l| * [ Ad * n_d * (uk+uj)/2 + Bd * (uk-uj)/dist ]
                // Coeff for uk: |l| * ( (Ad*n_d/2) + (Bd/dist) )
                // Coeff for uj: |l| * ( (Ad*n_d/2) - (Bd/dist) )
                float coeff_k = face_len * ((A_d * n_d * 0.5f) + (B_d / dist));
                float coeff_j = face_len * ((A_d * n_d * 0.5f) - (B_d / dist));

                entries(entry_ptr) = k_idx;
                values(entry_ptr) = coeff_k;
                diag_val += coeff_j;
                entry_ptr++;
              };

              // Dimension d=0 (X-direction) normals: left is -1, right is 1
              if (i > 0)
                add_face(i - 1, j, -1.0f, dy, dx);
              if (i < nb_x - 1)
                add_face(i + 1, j, 1.0f, dy, dx);
              if (j > 0)
                add_face(i, j - 1, 0.0f, dx, dy);
              if (j < nb_y - 1)
                add_face(i, j + 1, 0.0f, dx, dy);

              values(row_start) = diag_val;
              b(j_idx) = rhs_val;

              // Dirichlet BC override: Top boundary
              if (j == 0) {
                for (int k = row_map(j_idx); k < row_map(j_idx + 1); ++k) {
                  values(k) = (entries(k) == j_idx) ? 1.0f : 0.0f;
                }
                b(j_idx) = 1.0f; // Hot top
              }
            });

        auto u_next = solve_CG_sparse(A, b, total);
        Kokkos::deep_copy(u_n, u_next);
      }

      // --- VISUALIZATION ---
      ImGui::Checkbox("Running", &running);
      ImGui::ColorEdit3("Cold", cold);
      ImGui::ColorEdit3("Hot", hot);

      ImGui::Text("Delta time : %f", deltaTime);

      // Kokkos::parallel_for(
      //     "ApplyColor", total,
      //     KOKKOS_LAMBDA(const uint32_t id){
      // float v = u_n(id);
      // v = (v < 0.0f) ? 0.0f : (v > 1.0f ? 1.0f : v); // Clamp
      // colors[id] = Vector4f(1.0f, 0.0f, 0.0, 1.0f);
      // Vector4f(cold[0] * (1 - v) + hot[0] * v,
      // cold[1] * (1 - v) + hot[1] * v,
      // cold[2] * (1 - v) + hot[2] * v, 1.0f);
      // });
      //
    });
  }
  Kokkos::finalize();
  return 0;
}
