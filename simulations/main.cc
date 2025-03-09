#include "api/flim_api.hh"
#include "api/render/mesh.hh"
#include "api/render/mesh_utils.hh"
#include <Kokkos_Core.hpp>

using namespace Flim;

int main() {
  Kokkos::initialize();

  FlimAPI api = FlimAPI::init();
  {
    // this creates the buffer for the triangles
    Mesh mesh = MeshUtils::loadFromFile("resources/single_file/teddy.obj"); // or other
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
    scene.registerMesh(mesh, params);

    Instance &instance = scene.instantiate(mesh);

    // main loop
    api.run([&](float deltaTime) {
      (void)deltaTime;
    });
  }
  Kokkos::finalize();
  return 0;
}

