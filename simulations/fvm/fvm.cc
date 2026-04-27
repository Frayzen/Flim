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
    api.run([&](float deltaTime) {
      (void)deltaTime;
      const char *items[] = {"Triangles", "Bars", "Dots"};
      if (ImGui::Combo("Rendering type", ((int *)&(params.mode)), items,
                       IM_ARRAYSIZE(items))) {
        params.invalidate();
      }
      if (ImGui::SliderFloat3("Color", &color.x(), 0, 1)) {
        Vector4f toApply = color;
        Kokkos::parallel_for(
            "Apply color", colors.size(),
            KOKKOS_LAMBDA(const uint32_t id) { colors[id] = toApply; });
      }

      ImGui::Checkbox("Running", &running);
      if (ImGui::Button("Step") || running) {
      }
    });
  }
  Kokkos::finalize();
  return 0;
}
