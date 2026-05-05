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

Kokkos::View<float *> solve_CG(Kokkos::View<float **> matrix,
                               Kokkos::View<float *> f, int size) {
  Kokkos::View<float *> u("u", size);
  Kokkos::View<float *> r("r", size);
  Kokkos::View<float *> p("p", size);
  Kokkos::View<float *> Ap("Ap", size);

  Kokkos::deep_copy(u, 0.0f);
  Kokkos::deep_copy(r, f);
  Kokkos::deep_copy(p, r);

  float rr_old = KokkosBlas::dot(r, r);

  // Debug output
  printf("Initial residual: %e\n", sqrt(rr_old));

  for (int iter = 0; iter < 1000; iter++) {
    KokkosBlas::gemv("N", 1.0f, matrix, p, 0.0f, Ap);

    float pAp = KokkosBlas::dot(p, Ap);
    if (pAp == 0.0f)
      break; // Avoid division by zero

    float alpha = rr_old / pAp;
    KokkosBlas::axpy(alpha, p, u);
    KokkosBlas::axpy(-alpha, Ap, r);

    float rr_new = KokkosBlas::dot(r, r);

    if (iter % 100 == 0) {
      printf("Iter %d, residual: %e\n", iter, sqrt(rr_new));
    }

    if (sqrt(rr_new) < 1e-6) {
      printf("Converged after %d iterations\n", iter);
      break;
    }

    float beta = rr_new / rr_old;

    // FIXED: p = r + beta * p
    Kokkos::parallel_for(
        "update_p", size, KOKKOS_LAMBDA(int i) { p(i) = r(i) + beta * p(i); });

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

    auto total = nb_x * nb_y;
    Kokkos::View<float **> matrix("M", total, total);
    Kokkos::View<float *> f("f", total);

    // CRITICAL: Initialize to zero!
    Kokkos::deep_copy(matrix, 0.0f);
    Kokkos::deep_copy(f, 0.0f);

    Kokkos::parallel_for(
        "Create stencils",
        Kokkos::MDRangePolicy<Kokkos::Rank<2>>({0, 0}, {nb_x, nb_y}),
        KOKKOS_LAMBDA(const int i, const int j) {
          auto cur = j * nb_x + i;

          if (j == 0) {
            matrix(cur, cur) = 1.0f;
            f(cur) = 100.0f;
          } else if (j == nb_y - 1) {
            matrix(cur, cur) = 1.0f;
            f(cur) = 0.0f;
          } else {
            float sum = 0.0f;
            if (i > 0) {
              matrix(cur, cur - 1) = -1.0f;
              sum += 1.0f;
            }
            if (i < nb_x - 1) {
              matrix(cur, cur + 1) = -1.0f;
              sum += 1.0f;
            }
            if (j > 0) {
              matrix(cur, cur - nb_x) = -1.0f;
              sum += 1.0f;
            }
            if (j < nb_y - 1) {
              matrix(cur, cur + nb_x) = -1.0f;
              sum += 1.0f;
            }
            matrix(cur, cur) = sum;
            f(cur) = 0.0f;
          }
        });
    Kokkos::fence();

    auto solution = solve_CG(matrix, f, total);

    // Debug: Print some solution values
    auto solution_host =
        Kokkos::create_mirror_view_and_copy(Kokkos::HostSpace(), solution);
    printf("\nSolution sample (bottom to top):\n");
    for (int j = 0; j < nb_y; j++) {
      int i = nb_x / 2; // Middle column
      int idx = j * nb_x + i;
      printf("j=%d (row %d): %f\n", j, idx / nb_x, solution_host(idx));
    }

    float scale = 1;
    bool set = false;
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

      float scale_v = scale;
      bool set_v = set;

      Kokkos::parallel_for(
          "Apply color", colors.size(), KOKKOS_LAMBDA(const uint32_t id) {
            // colors[id] = toApply;
            float v = scale_v;
            if (!set_v)
              v *= solution(id);
            // v = Kokkos::min<float>(v, 0.1);
            colors[id] = Vector4f(v, v, v, 1.0f);
          });
      // }

      ImGui::Checkbox("Running", &running);
      if (ImGui::Button("Step") || running) {
      }
    });
  }
  Kokkos::finalize();
  return 0;
}
