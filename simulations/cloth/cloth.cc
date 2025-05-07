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
#include <decl/Kokkos_Declare_OPENMP.hpp>
#include <imgui.h>
#include <impl/Kokkos_Profiling.hpp>
#include <setup/Kokkos_Setup_HIP.hpp>

using namespace Flim;

KOKKOS_FUNCTION
static Kokkos::pair<Vector3f, Vector3f> apply_constraint(Vector3f p0,
                                                         Vector3f p1, float w0,
                                                         float w1,
                                                         float inv_dt_sq) {
  Vector3f e(p0 - p1);
  auto curlen = e.norm();
  if (curlen == 0) {
    return Kokkos::make_pair(p0, p1);
  }
  auto ne = e / curlen;
  auto dlen = curlen - 1;
  auto lambda = -dlen / (w0 + w1 + inv_dt_sq);
  return Kokkos::make_pair(lambda * w0 * ne, -lambda * w1 * ne);
}

int main() {
  Kokkos::initialize();
  FlimAPI api = FlimAPI::init();
  {

    static int nb_x = 25;
    static int nb_y = 15;

    Mesh mesh = MeshUtils::createGrid(1, nb_x, nb_y);
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
    weights.h_view(0, 0) = 0;
    weights.h_view(nb_x - 1, 0) = 0;
    weights.sync_device();

    scene.camera.controls = true;
    scene.camera.speed = 5;
    scene.camera.sensivity = 5;

    // PARAMETERS

    float g = 9;
    float damping = 0.9999;
    float k = 100; // stiffness
    int substep = 4;

    bool running;
    api.run([&](float deltaTime) {
      ImGui::SliderFloat("K", &k, 1, 5000);
      ImGui::SliderInt("Amount substep", &substep, 1, 10);
      ImGui::SliderFloat("Damping", &damping, 0.9, 0.9999999);
      ImGui::SliderFloat("G", &g, 0, 20);

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

        float dt = deltaTime / substep;
        for (int _ = 0; _ < substep; _++) {
          // UPDATE POS
          Kokkos::parallel_for(
              "Update position", Kokkos::MDRangePolicy({0, 0}, {nb_x, nb_y}),
              KOKKOS_LAMBDA(const int i, const int j) {
                pts(i, j).pos += dt * vels(i, j);
              });
          Kokkos::fence("Wait update pos");

          // APPLY CONSTRAINT
          float alpha = 1 / k;
          float inv_dt_sq = alpha / (dt * dt);
          Kokkos::parallel_for(
              "Update horizontal constraints",
              Kokkos::MDRangePolicy({0, 0}, {nb_x, nb_y - 1}),
              KOKKOS_LAMBDA(const int i, const int j) {
                auto deltas = apply_constraint(
                    pts(i, j).pos, pts(i, j + 1).pos, weights.d_view(i, j),
                    weights.d_view(i, j + 1), inv_dt_sq);
                pts(i, j).pos += deltas.first;
                pts(i, j + 1).pos += deltas.second;
              });
          Kokkos::fence("Wait horizontal constraints");

          Kokkos::parallel_for(
              "Update vertical constraints",
              Kokkos::MDRangePolicy({0, 0}, {nb_x - 1, nb_y}),
              KOKKOS_LAMBDA(const int i, const int j) {
                auto deltas = apply_constraint(
                    pts(i, j).pos, pts(i + 1, j).pos, weights.d_view(i, j),
                    weights.d_view(i + 1, j), inv_dt_sq);
                pts(i, j).pos += deltas.first;
                pts(i + 1, j).pos += deltas.second;
              });
          Kokkos::fence("Wait vertical constraints");
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
