#include "api/flim_api.hh"
#include "api/parameters/render_params.hh"
#include "api/render/mesh.hh"
#include "api/render/mesh_utils.hh"
#include "api/transform.hh"
#include "api/tree/instance.hh"
#include <Eigen/src/Core/Matrix.h>
#include <Kokkos_Core.hpp>
#include <Kokkos_Random.hpp>
#include <decl/Kokkos_Declare_OPENMP.hpp>
#include <imgui.h>
#include <impl/Kokkos_Profiling.hpp>
#include <iostream>
#include <setup/Kokkos_Setup_HIP.hpp>
#include <unistd.h>

using namespace Flim;

int main() {
  Kokkos::initialize();

  FlimAPI api = FlimAPI::init();
  // this creates the buffer for the triangles
  Mesh mesh =
      MeshUtils::loadFromFile("resources/single_file/dragon.obj"); // or other
  // by default, mesh has a material with fixed color (ambient, diffuse,
  // specular) later on, material can have per vertex or per triangle color
  // later on, material can have textures instead of colors
  // this will be stored as flags in a single shader (with unbound values such
  // as textures) in any way, the basic color will be given as a uniform and
  // applied (set to 0 to discard it) the possibility to UPDATE THE MATERIAL
  // is here, no buffer are actually created until next line

  Scene &scene = api.getScene();

  // can specify render params (shaders) but default ones are using previous
  // explanations
  RenderParams params = RenderParams::DefaultParams(mesh, scene.camera);
  params.useBackfaceCulling = true;

  const Renderer &rd = scene.registerMesh(mesh, params);

  Instance &instance = scene.instantiate(mesh);
  instance.transform.scale = Vector3f(3, 3, 3);

  scene.camera.controls = true;
  scene.camera.speed = 20;
  scene.camera.sensivity = 5;

  static int ecol = 1;
  api.setupGraphics();
  Kokkos::View<Vertex *> vertices =
      rd.getAttributeBufferView<Vertex>(BINDING_DEFAULT_VERTICES_ATTRIBUTES);

  api.run([&](float deltaTime) {
    ImGui::InputInt("Amount of ecol", &ecol);
    if (ImGui::Button("Apply ecol")) {
      std::cout << "OK " + std::to_string(deltaTime) << std::endl;
    }
    Kokkos::fence("Wait for move");
  });
  Kokkos::finalize();
  return 0;
}
