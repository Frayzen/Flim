#include "api/flim_api.hh"
#include "api/parameters.hh"
#include "api/render/mesh_utils.hh"
#include "api/scene.hh"

using namespace Flim;

int main() {
  Flim::FlimAPI api = FlimAPI::init();
  Scene &scene = api.getScene();
  Renderer renderer = {
      Shader("shaders/shader.vert.spv"),
      Shader("shaders/shader.frag.spv"),
  };
  scene.defaultRenderer(&renderer);
  Mesh sphere = MeshUtils(scene).createSphere(1);
  scene.instantiate(sphere);
  return api.run();
}
