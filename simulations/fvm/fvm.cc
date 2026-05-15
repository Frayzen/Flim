#include "api/flim_api.hh"
#include "api/parameters/base_params.hh"
#include "api/parameters/render_params.hh"
#include "api/render/mesh.hh"
#include "api/render/mesh_utils.hh"
#include "api/tree/instance.hh"
#include "kokkos/renderer_accesser.hh"
#include "navier_stokes_solver.hh"
#include "vulkan/buffers/params_utils.hh"
#include "vulkan/rendering/renderer.hh"

using namespace Flim;

#include <KokkosBlas1_axpby.hpp>
#include <KokkosBlas1_dot.hpp>
#include <KokkosSparse_CrsMatrix.hpp>
#include <KokkosSparse_spmv.hpp>

int main() {
  Kokkos::initialize();
  FlimAPI api = FlimAPI::init();
  {

    const int nb_x = 50;
    const int nb_y = 50;
    const int total = nb_x * nb_y;
    const float L_x = 1.0f;
    const float L_y = 1.0f;
    const float dx = L_x / (nb_x - 1);
    const float dy = L_y / (nb_y - 1);
    NavierStokesSolver ns(nb_x, nb_y, 1.0f, 1.0f);
    FluidParams fp = {0.001f, 1.0f, 0.01f}; // Low viscosity = High Reynolds

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

    params.mode = RenderMode::RENDERER_MODE_TRIS;
    params.useBackfaceCulling = false;
    const Renderer &rd = scene.registerMesh(mesh, params);
    scene.instantiate(mesh);

    api.setupGraphics();

    scene.camera.controls = true;
    scene.camera.speed = 5;
    scene.camera.sensivity = 5;
    scene.camera.transform.position = Vector3f(0, nb_y, (nb_x + nb_y) / 2.0f);

    auto colors = getAttributeBufferView<Vector4f>(rd, 1);

    bool running = false;
    float cold[3] = {0.1f, 0.1f, 1.0f};
    float hot[3] = {1.0f, 0.1f, 0.1f};
    static float max = 1.0f;
    static float min = 1.0f;

    float dt = 0.0001;

    api.run([&](float deltaTime) {
      static float sumtime = 0.0f;
      sumtime += deltaTime;

      auto u_n = ns.getU();
      if (ImGui::Button("Step") || running) {
        // --- ASSEMBLE MATRIX AND RHS ---
        float curtime = sumtime;
        ns.step(fp, dt);
        typedef Kokkos::MinMax<float>::value_type MinMax;
        MinMax minmax;
        u_n = ns.getU();
        Kokkos::parallel_reduce(
            "MinMaxReduce", u_n.size(),
            KOKKOS_LAMBDA(uint32_t i, MinMax &m) {
              if (u_n[i] < m.min_val)
                m.min_val = u_n[i];
              if (u_n[i] > m.max_val)
                m.max_val = u_n[i];
            },
            Kokkos::MinMax<float>(minmax));
        min = minmax.min_val;
        max = minmax.max_val;
      }

      // --- VISUALIZATION ---
      ImGui::Checkbox("Running", &running);
      ImGui::ColorEdit3("Cold", cold);
      ImGui::ColorEdit3("Hot", hot);

      ImGui::Text("Delta time : %f", dt);
      ImGui::Text("MAX VAL: %f", max);
      ImGui::Text("MIN VAL: %f", min);

      float cur_min = min;
      float cur_max = max;
      Vector4f hotv = Vector4f(hot[0], hot[1], hot[2], 1.0f);
      Vector4f coldv = Vector4f(cold[0], cold[1], cold[2], 1.0f);
      Kokkos::parallel_for(
          "ApplyColor", total, KOKKOS_LAMBDA(const uint32_t id) {
            float v = (u_n(id) - cur_min) / (cur_max - cur_min);
            colors[id] = v * hotv + (1 - v) * coldv;
          });
    });
  }
  Kokkos::finalize();
  return 0;
}
