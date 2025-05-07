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

    Mesh mesh = MeshUtils::createGrid(0.2, nb_x, nb_y);
    auto &scene = api.getScene();
    RenderParams params = RenderParams::DefaultParams(mesh, scene.camera);
    params.useBackfaceCulling = false;
    params.mode = RenderMode::RENDERER_MODE_LINE;
    const Renderer &rd = scene.registerMesh(mesh, params);
    Instance &c = scene.instantiate(mesh);

    api.setupGraphics();
    Kokkos::View<Vertex *> vertices =
        rd.getAttributeBufferView<Vertex>(BINDING_DEFAULT_VERTICES_ATTRIBUTES);
    Kokkos::View<Vertex **> pts(vertices.data(), nb_y, nb_x);
    Kokkos::View<Vertex **> oldPos("Old points", nb_y, nb_x);
    Kokkos::View<Vector3f **> vels("Velocities", nb_y, nb_x);
    Kokkos::DualView<float **> weights("Weights", nb_y,
                                       nb_x); // inverse of the mass

    // Pin the top corners
    weights.h_view(0, 0) = 0;
    weights.h_view(0, nb_x - 1) = 0;
    weights.modify_host();
    weights.sync_device();

    scene.camera.controls = true;
    scene.camera.speed = 5;
    scene.camera.sensivity = 5;

    // PARAMETERS

    float side = 10;
    float g = 9;
    float damping = 0.9999;
    float k = 1000; // stiffness
    int substep = 4;

    static float maxDistMove = 0.5;
    api.run([&](float deltaTime) {
      (void)deltaTime;

      Kokkos::deep_copy(oldPos, pts);

      Kokkos::parallel_for(
          "Apply gravity", Kokkos::MDRangePolicy({0, 0}, {nb_x, nb_y}),
          KOKKOS_LAMBDA(const int i, const int j) {
            vels(j, i).y() += deltaTime * g * weights.d_view(j, i);
          });
      Kokkos::fence("Wait gravity");

      float dt = deltaTime / substep;
      for (int _ = 0; _ < substep; _++) {
        // UPDATE POS
        Kokkos::parallel_for(
            "Update position", Kokkos::MDRangePolicy({0, 0}, {nb_x, nb_y}),
            KOKKOS_LAMBDA(const int i, const int j) {
              pts(j, i).pos += dt * vels(j, i);
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
                  pts(j, i).pos, pts(j + 1, i).pos, weights.d_view(j, i),
                  weights.d_view(j + 1, i), inv_dt_sq);
              pts(j, i).pos += deltas.first;
              pts(j + 1, i).pos += deltas.second;
            });
        Kokkos::fence("Wait horizontal constraints");

        Kokkos::parallel_for(
            "Update vertical constraints",
            Kokkos::MDRangePolicy({0, 0}, {nb_x - 1, nb_y}),
            KOKKOS_LAMBDA(const int i, const int j) {
              auto deltas = apply_constraint(
                  pts(j, i).pos, pts(j, i + 1).pos, weights.d_view(j, i),
                  weights.d_view(j, i + 1), inv_dt_sq);
              pts(j, i).pos += deltas.first;
              pts(j, i + 1).pos += deltas.second;
            });
        Kokkos::fence("Wait vertical constraints");
      }

      // Apply damping
      Kokkos::parallel_for(
          "Apply damping", Kokkos::MDRangePolicy({0, 0}, {nb_x, nb_y}),
          KOKKOS_LAMBDA(const int i, const int j) {
            vels(j, i) = damping * pts(j, i).pos - oldPos(j, i).pos / deltaTime;
          });
      Kokkos::fence("Wait damping");
    });
  }
  Kokkos::finalize();
  return 0;
}
