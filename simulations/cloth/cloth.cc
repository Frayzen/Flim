#include "api/flim_api.hh"
#include "api/parameters/base_params.hh"
#include "api/parameters/render_params.hh"
#include "api/render/mesh.hh"
#include "api/render/mesh_utils.hh"
#include "api/tree/instance.hh"
#include "vulkan/rendering/renderer.hh"
#include <Eigen/src/Core/Matrix.h>
#include <Kokkos_Core.hpp>
#include <Kokkos_DualView.hpp>
#include <Kokkos_Macros.hpp>
#include <Kokkos_Pair.hpp>
#include <cmath>
#include <decl/Kokkos_Declare_OPENMP.hpp>
#include <imgui.h>
#include <impl/Kokkos_Profiling.hpp>
#include <setup/Kokkos_Setup_HIP.hpp>

using namespace Flim;

KOKKOS_FUNCTION
static Vector3f apply_constraint(Vector3f p0, Vector3f p1, float w0, float w1,
                                 float inv_dt_sq, float length) {
  Vector3f e(p0 - p1);
  auto curlen = e.norm();
  if (curlen == 0) {
    return Vector3f(0, 0, 0);
  }
  auto ne = e / curlen;
  auto dlen = curlen - length;
  auto lambda = -dlen / (w0 + w1 + inv_dt_sq);
  return lambda * w0 * ne;
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
    RenderParams params = RenderParams::DefaultParams(mesh, scene.camera);
    params.useBackfaceCulling = false;
    params.mode = RenderMode::RENDERER_MODE_LINE;
    const Renderer &rd = scene.registerMesh(mesh, params);
    Instance &c = scene.instantiate(mesh);

    api.setupGraphics();
    Kokkos::View<Vertex *> vertices =
        rd.getAttributeBufferView<Vertex>(BINDING_DEFAULT_VERTICES_ATTRIBUTES);
    Kokkos::View<Vertex **> pts(vertices.data(), nb_x, nb_y);
    Kokkos::View<Vertex **> initPos("Init points", nb_x, nb_y);
    Kokkos::deep_copy(initPos, pts);
    Kokkos::View<Vertex **> oldPos("Old points", nb_x, nb_y);
    Kokkos::View<Vector3f **> vels("Velocities", nb_x, nb_y);

    // inverse of the mass
    Kokkos::DualView<float **> weights("Weights", nb_x, nb_y);

    weights.modify_host();
    for (int i = 0; i < nb_x; i++) {
      for (int j = 0; j < nb_y; j++)
        weights.h_view(i, j) = 1;
    }
    // Pin the top corners
    weights.h_view(0, nb_y - 1) = 0;
    weights.h_view(nb_x - 1, nb_y - 1) = 0;
    weights.sync_device();

    scene.camera.controls = true;
    scene.camera.speed = 5;
    scene.camera.sensivity = 5;

    // PARAMETERS

    float g = 9;
    float damping = 0.99;
    float k = 100; // stiffness
    int substep = 4;

    bool running = false;
    api.run([&](float deltaTime) {
      ImGui::SliderFloat("K", &k, 100, 10000);
      ImGui::SliderInt("Amount substep", &substep, 1, 10);
      ImGui::SliderFloat("Damping", &damping, 0.9, 0.9999999);
      ImGui::SliderFloat("G", &g, 0, 20);

      const char *items[] = {"Triangles", "Bars", "Dots"};
      if (ImGui::Combo("Rendering type", ((int *)&(params.mode)), items,
                       IM_ARRAYSIZE(items))) {
        params.invalidate();
      }

      if (ImGui::Button("Reset")) {
        Kokkos::deep_copy(pts, initPos);
        Kokkos::parallel_for(
            "Apply gravity", Kokkos::MDRangePolicy({0, 0}, {nb_x, nb_y}),
            KOKKOS_LAMBDA(const int i, const int j) {
              vels(i, j) = Vector3f(0, 0, 0);
            });
        Kokkos::fence("Wait reset");
      }

      ImGui::Checkbox("Running", &running);
      if (ImGui::Button("Step") || running) {
        // SAVE INITAL POINTS
        Kokkos::deep_copy(oldPos, pts);

        // APPLY GRAVITY
        Kokkos::parallel_for(
            "Apply gravity", Kokkos::MDRangePolicy({0, 0}, {nb_x, nb_y}),
            KOKKOS_LAMBDA(const int i, const int j) {
              vels(i, j).y() -= deltaTime * g * weights.d_view(i, j);
            });
        Kokkos::fence("Wait gravity");

        float compliance = 1 / k;
        float dt = deltaTime / substep;
        float inv_dt_sq = compliance / (dt * dt);
        for (int _ = 0; _ < substep; _++) {
          // UPDATE POS
          Kokkos::parallel_for(
              "Update position", Kokkos::MDRangePolicy({0, 0}, {nb_x, nb_y}),
              KOKKOS_LAMBDA(const int i, const int j) {
                pts(i, j).pos += dt * vels(i, j);
              });
          Kokkos::fence("Wait update pos");

          // APPLY CONSTRAINT
          Kokkos::parallel_for(
              "Update constraints", Kokkos::MDRangePolicy({0, 0}, {nb_x, nb_y}),
              KOKKOS_LAMBDA(const int i, const int j) {
                Vector3f delta;
#define UPDATE_CONSTRAINT_NEIGHBOUR(X, Y, Length)                              \
  if (X >= 0 && X < nb_x && Y >= 0 && Y < nb_y) {                              \
    delta =                                                                    \
        apply_constraint(pts(i, j).pos, pts(X, Y).pos, weights.d_view(i, j),   \
                         weights.d_view(X, Y), inv_dt_sq, Length);             \
    pts(i, j).pos += delta;                                                    \
  }
                UPDATE_CONSTRAINT_NEIGHBOUR(i, j + 1, side_length)
                UPDATE_CONSTRAINT_NEIGHBOUR(i, j - 1, side_length)
                UPDATE_CONSTRAINT_NEIGHBOUR(i - 1, j, side_length)
                UPDATE_CONSTRAINT_NEIGHBOUR(i + 1, j, side_length)

                UPDATE_CONSTRAINT_NEIGHBOUR(i + 1, j + 1, diag_length)
                UPDATE_CONSTRAINT_NEIGHBOUR(i + 1, j - 1, diag_length)
                UPDATE_CONSTRAINT_NEIGHBOUR(i - 1, j + 1, diag_length)
                UPDATE_CONSTRAINT_NEIGHBOUR(i - 1, j - 1, diag_length)
              });
          Kokkos::fence("Wait constraints");
        }
        // Apply damping
        Kokkos::parallel_for(
            "Apply damping", Kokkos::MDRangePolicy({0, 0}, {nb_x, nb_y}),
            KOKKOS_LAMBDA(const int i, const int j) {
              vels(i, j) =
                  damping * (pts(i, j).pos - oldPos(i, j).pos) / deltaTime;
            });
        Kokkos::fence("Wait damping");
      }
    });
  }
  Kokkos::finalize();
  return 0;
}
